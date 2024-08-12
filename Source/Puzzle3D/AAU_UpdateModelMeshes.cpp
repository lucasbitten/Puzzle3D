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
#include "InnerMesh.h"

#include "Editor/UnrealEd/Public/Editor.h"
#include "PuzzleModel.h"


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
                int32 innerMeshCount = 0;

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
                            USCS_Node* NewNode = nullptr;
                            if (StaticMeshComponent->GetName().Contains(TEXT("SphereAux")))
                            {
                                FVector Location = StaticMeshComponent->GetRelativeLocation();
                                FRotator Rotation = StaticMeshComponent->GetRelativeRotation();
                                FVector Scale = StaticMeshComponent->GetRelativeScale3D();

                                FString NewName = FString::Printf(TEXT("SphereAux_%d"), innerMeshCount++);
                                NewNode = SCS->CreateNode(UInnerMesh::StaticClass(), *NewName);

                                UInnerMesh* NewInnerMesh = Cast<UInnerMesh>(NewNode->ComponentTemplate);

                                if (!NewInnerMesh)
                                {
                                    UE_LOG(LogTemp, Error, TEXT("Failed to create NewInnerMesh for %s"), *StaticMeshComponent->GetName());
                                    continue;
                                }

                                NewInnerMesh->SetRelativeLocation(Location);
                                NewInnerMesh->SetRelativeRotation(Rotation);
                                NewInnerMesh->SetRelativeScale3D(Scale);

                            }
                            else
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
                                NewNode = SCS->CreateNode(UPuzzlePiecesComponent::StaticClass(), *NewName);
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
                                }

                                NewPuzzlePartComponent->SetIsShell(IsShell);
                            }

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

void UAAU_UpdateModelMeshes::RemoveReorientedPlanes()
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

                // Validar e garantir que o nó raiz padrão esteja presente
                SCS->ValidateSceneRootNodes();

                // Lista temporária para armazenar os novos nós na ordem desejada
                TArray<USCS_Node*> NewNodes;

                // Iterar pelos nós do SCS
                TArray<USCS_Node*> Nodes = SCS->GetAllNodes();
                int32 count = 0;

                // Guardar nós para remoção
                TArray<USCS_Node*> NodesToRemove;

                for (USCS_Node* Node : Nodes)
                {
                    if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Node->ComponentTemplate))
                    {
                        // Verifique a validade da malha original
                        if (StaticMeshComponent->GetStaticMesh() && StaticMeshComponent->GetStaticMesh()->IsMeshDescriptionValid(0))
                        {
                            bool IsShell = StaticMeshComponent->GetName().Contains(TEXT("Shell"));
                            if (IsShell)
                            {
                                // Salve as propriedades da child
                                UStaticMesh* Mesh = StaticMeshComponent->GetStaticMesh();
                                FVector ChildLocation = StaticMeshComponent->GetRelativeLocation();
                                FRotator ChildRotation = StaticMeshComponent->GetRelativeRotation();
                                FVector ChildScale = StaticMeshComponent->GetRelativeScale3D();

                                // Criar um novo nó com UPuzzlePiecesComponent para a child
                                FString NewChildName = FString::Printf(TEXT("PuzzlePart_Shell"));
                                USCS_Node* NewShellNode = SCS->CreateNode(UPuzzlePiecesComponent::StaticClass(), *NewChildName);
                                UPuzzlePiecesComponent* NewPuzzlePartComponent = Cast<UPuzzlePiecesComponent>(NewShellNode->ComponentTemplate);

                                if (NewPuzzlePartComponent)
                                {
                                    // Copie as propriedades
                                    NewPuzzlePartComponent->SetStaticMesh(Mesh);
                                    NewPuzzlePartComponent->SetRelativeLocation(ChildLocation);
                                    NewPuzzlePartComponent->SetRelativeRotation(ChildRotation);
                                    NewPuzzlePartComponent->SetRelativeScale3D(ChildScale);

                                    // Definir o perfil de colisão padrão
                                    NewPuzzlePartComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));

                                    NewPuzzlePartComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore); // Supondo que Pieces é ECC_GameTraceChannel1
                                    NewPuzzlePartComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Block); // Supondo que Shell é ECC_GameTraceChannel2
                                    NewPuzzlePartComponent->SetIsShell(true);

                                    // Adicionar o novo nó à lista temporária
                                    NewNodes.Add(NewShellNode);

                                    // Marcar o nó para remoção
                                    NodesToRemove.Add(Node);
                                }
                            }
                            else if (StaticMeshComponent->GetName().Contains(TEXT("Reorient_New")))
                            {
                                // Salve as propriedades de transformação
                                FVector Location = StaticMeshComponent->GetRelativeLocation();
                                FRotator Rotation = StaticMeshComponent->GetRelativeRotation();
                                FVector Scale = StaticMeshComponent->GetRelativeScale3D();

                                // Criar um novo nó de componente de cena
                                FString NewComponentName = FString::Printf(TEXT("PuzzlePart_%d_Parent"), count);

                                USCS_Node* NewSceneNode = SCS->CreateNode(USceneComponent::StaticClass(), *NewComponentName);
                                USceneComponent* NewSceneComponent = Cast<USceneComponent>(NewSceneNode->ComponentTemplate);

                                if (NewSceneComponent)
                                {
                                    // Copie as propriedades de transformação
                                    NewSceneComponent->SetRelativeLocation(Location);
                                    NewSceneComponent->SetRelativeRotation(Rotation);
                                    NewSceneComponent->SetRelativeScale3D(Scale);

                                    // Realocar filhos para o novo nó de componente de cena
                                    TArray<USCS_Node*> ChildNodes = Node->GetChildNodes();
                                    for (USCS_Node* ChildNode : ChildNodes)
                                    {
                                        UStaticMeshComponent* ChildStaticMeshComponent = Cast<UStaticMeshComponent>(ChildNode->ComponentTemplate);
                                        if (ChildStaticMeshComponent)
                                        {
                                            // Salve as propriedades da child
                                            UStaticMesh* Mesh = ChildStaticMeshComponent->GetStaticMesh();
                                            FVector ChildLocation = ChildStaticMeshComponent->GetRelativeLocation();
                                            FRotator ChildRotation = ChildStaticMeshComponent->GetRelativeRotation();
                                            FVector ChildScale = ChildStaticMeshComponent->GetRelativeScale3D();

                                            // Criar um novo nó com UPuzzlePiecesComponent para a child
                                            FString NewChildName = FString::Printf(TEXT("PuzzlePart_%d"), count);
                                            USCS_Node* NewChildNode = SCS->CreateNode(UPuzzlePiecesComponent::StaticClass(), *NewChildName);
                                            UPuzzlePiecesComponent* NewPuzzlePartComponent = Cast<UPuzzlePiecesComponent>(NewChildNode->ComponentTemplate);

                                            if (NewPuzzlePartComponent)
                                            {
                                                // Copie as propriedades
                                                NewPuzzlePartComponent->SetStaticMesh(Mesh);
                                                NewPuzzlePartComponent->SetRelativeLocation(ChildLocation);
                                                NewPuzzlePartComponent->SetRelativeRotation(ChildRotation);
                                                NewPuzzlePartComponent->SetRelativeScale3D(ChildScale);

                                                // Definir o perfil de colisão padrão
                                                NewPuzzlePartComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));

                                                // Ajustar canais específicos
                                                NewPuzzlePartComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Block); // Supondo que Pieces é ECC_GameTraceChannel1
                                                NewPuzzlePartComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore); // Supondo que Shell é ECC_GameTraceChannel2
                                                NewPuzzlePartComponent->SetIsShell(false);

                                                // Verificar antes de anexar o novo nó de componente de cena ao novo nó de PuzzlePart
                                                if (ensure(NewSceneNode) && ensure(NewChildNode))
                                                {
                                                    NewSceneNode->AddChildNode(NewChildNode);
                                                }

                                                // Marcar o nó filho para remoção
                                                NodesToRemove.Add(ChildNode);
                                            }
                                        }
                                    }

                                    // Adicionar o novo nó à lista temporária
                                    NewNodes.Add(NewSceneNode);

                                    // Marcar o nó original para remoção
                                    NodesToRemove.Add(Node);
                                }
                            }
                            count++;
                        }
                    }
                }

                // Remover os nós marcados
                for (USCS_Node* NodeToRemove : NodesToRemove)
                {
                    SCS->RemoveNode(NodeToRemove);
                }

                // Criar e adicionar o nó dummy após remover os nós antigos
                USCS_Node* DummyRootNode = SCS->CreateNode(USceneComponent::StaticClass(), TEXT("DummyRootNode"));
                SCS->AddNode(DummyRootNode);

                // Adicionar os novos nós ao DummyRootNode na ordem correta
                for (USCS_Node* NewNode : NewNodes)
                {
                    if (ensure(DummyRootNode) && ensure(NewNode))
                    {
                        DummyRootNode->AddChildNode(NewNode);
                    }
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
