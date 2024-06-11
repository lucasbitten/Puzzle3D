// Fill out your copyright notice in the Description page of Project Settings.


#include "AAU_UpdateModelMeshes.h"
#include "EditorUtilityLibrary.h"
#include "Engine/Blueprint.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/PackageName.h"
#include "PuzzlePiecesComponent.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"

#include "Editor/UnrealEd/Public/Editor.h"


void UAAU_UpdateModelMeshes::AddPiecesComponentToMeshes()
{
    // Obtenha os assets selecionados
    TArray<FAssetData> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssetData();

    for (const FAssetData& AssetData : SelectedAssets)
    {
        if (UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset()))
        {
            // Obter a SimpleConstructionScript do blueprint
            USimpleConstructionScript* SCS = Blueprint->SimpleConstructionScript;
            if (SCS)
            {
                // Modifique o blueprint
                Blueprint->Modify();

                // Iterar pelos nós do SCS
                TArray<USCS_Node*> Nodes = SCS->GetAllNodes();

                int32 count = 0;
                FVector ShellRelativePosition = FVector::Zero();
                TArray<USCS_Node*> NodesToRemove;
                for (USCS_Node* Node : Nodes)
                {
                    UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Node->ComponentTemplate);
                    if (StaticMeshComponent)
                    {
                        // Verifique a validade da malha original
                        if (StaticMeshComponent->GetStaticMesh() && StaticMeshComponent->GetStaticMesh()->IsMeshDescriptionValid(0))
                        {
                            bool IsShell = StaticMeshComponent->GetName().Contains(TEXT("Shell"));

                            // Salve as propriedades que você deseja manter
                            UStaticMesh* Mesh = StaticMeshComponent->GetStaticMesh();
                            FVector Location = StaticMeshComponent->GetRelativeLocation();
                            FRotator Rotation = StaticMeshComponent->GetRelativeRotation();
                            FVector Scale = StaticMeshComponent->GetRelativeScale3D();

                            FString NewName;
                            if (IsShell)
                            {
                                NewName = FString::Printf(TEXT("PuzzlePart_Shell"));
                            }
                            else
                            {
                                NewName = FString::Printf(TEXT("PuzzlePart_%d"), count++);
                            }

                            // Crie um novo nó no SCS com o novo componente
                            USCS_Node* NewNode = SCS->CreateNode(UPuzzlePiecesComponent::StaticClass(), *NewName);
                            UPuzzlePiecesComponent* NewPuzzlePartComponent = Cast<UPuzzlePiecesComponent>(NewNode->ComponentTemplate);

                            if (!NewPuzzlePartComponent)
                            {
                                UE_LOG(LogTemp, Error, TEXT("Failed to create PuzzlePiecesComponent for %s"), *StaticMeshComponent->GetName());
                                continue;
                            }

                            // Copie as propriedades
                            NewPuzzlePartComponent->SetStaticMesh(Mesh);
                            NewPuzzlePartComponent->SetRelativeLocation(Location);
                            NewPuzzlePartComponent->SetRelativeRotation(Rotation);
                            NewPuzzlePartComponent->SetRelativeScale3D(Scale);
                            NewPuzzlePartComponent->SetInitialRelativePosition(NewPuzzlePartComponent->GetRelativeLocation());

                            // Definir o perfil de colisão padrão
                            NewPuzzlePartComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));

                            if (IsShell)
                            {
                                // Ajustar canais específicos
                                NewPuzzlePartComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore); // Supondo que Pieces é ECC_GameTraceChannel1
                                NewPuzzlePartComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Block); // Supondo que Shell é ECC_GameTraceChannel2
                                ShellRelativePosition = StaticMeshComponent->GetRelativeLocation();
                            }
                            else
                            {
                                // Ajustar canais específicos
                                NewPuzzlePartComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Block); // Supondo que Pieces é ECC_GameTraceChannel1
                                NewPuzzlePartComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore); // Supondo que Shell é ECC_GameTraceChannel2
                                NewPuzzlePartComponent->SetShellRelativePosition(ShellRelativePosition);
                            }

                            NewPuzzlePartComponent->SetIsShell(IsShell);

                            // Encontre o nó pai original
                            USCS_Node* ParentNode = nullptr;
                            if (Node->ParentComponentOrVariableName != NAME_None)
                            {
                                for (USCS_Node* PotentialParentNode : Nodes)
                                {
                                    if (PotentialParentNode->GetVariableName() == Node->ParentComponentOrVariableName)
                                    {
                                        ParentNode = PotentialParentNode;
                                        break;
                                    }
                                }
                            }

                            // Anexar o novo nó ao pai do nó original
                            if (ParentNode)
                            {
                                NewNode->SetParent(ParentNode);
                                ParentNode->AddChildNode(NewNode);
                            }
                            else
                            {
                                SCS->AddNode(NewNode);
                            }

                            // Marcar o nó original para remoção
                            NodesToRemove.Add(Node);
                        }
                        else
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Original mesh description is invalid for component %s"), *StaticMeshComponent->GetName());
                        }
                    }
                }

                // Remover os nós antigos
                for (USCS_Node* NodeToRemove : NodesToRemove)
                {
                    SCS->RemoveNode(NodeToRemove);
                }

                // Compilar o blueprint para aplicar as mudanças
                FKismetEditorUtilities::CompileBlueprint(Blueprint);

                // Marque o blueprint como modificado e salve as mudanças
                FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
                FString PackageName = AssetData.PackageName.ToString();
                UPackage* Package = FindPackage(nullptr, *PackageName);
                if (Package)
                {
                    Package->SetDirtyFlag(true);
                    FAssetRegistryModule::AssetCreated(Package);
                }
            }
        }
    }

    // Atualize os editores
    GEditor->RedrawAllViewports();
}
