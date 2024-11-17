// Fill out your copyright notice in the Description page of Project Settings.

#if WITH_EDITOR

#include "AAU_UpdateModelMeshes.h"
#include "EditorUtilityLibrary.h"
#include "Engine/Blueprint.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/PackageName.h"
#include "./Puzzle3D/PuzzlePiecesComponent.h"
#include "./Puzzle3D/PuzzlePieceParentComponent.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"
#include "./Puzzle3D/InnerMesh.h"

#include "Editor/UnrealEd/Public/Editor.h"
#include "./Puzzle3D/PuzzleModel.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "Editor.h"
#include "Editor/UnrealEd/Public/EditorActorFolders.h"
#include "Editor/EditorEngine.h"
#include "EngineUtils.h"
#include "DatasmithSceneActor.h"
#include "DatasmithScene.h"
#include "DatasmithAssetImportData.h"
#include "DatasmithImporter.h"
#include "DatasmithBlueprintLibrary.h"
#include "ActorFactoryDatasmithScene.h"
#include "ExternalSourceModule.h"
#include "Utility/DatasmithImporterUtils.h"
#include "Editor/UnrealEd/Private/Editor/ActorPositioning.h"

void UAAU_UpdateModelMeshes::GenerateModelBlueprint()
{
    using namespace UE::DatasmithImporter;

    FString CurvePath = TEXT("/Game/LerpCurve.LerpCurve");

    UCurveFloat* AlphaCurve = LoadCurveFromPath(CurvePath);

    if (!AlphaCurve)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load AlphaCurve from path: %s"), *CurvePath);
        return;
    }


    // Obtenha os assets selecionados
    TArray<UObject*> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssets();

    for (UObject* AssetObject : SelectedAssets)
    {
        // Verifique se o asset é um UDatasmithScene
        if (UDatasmithScene* DatasmithScene = Cast<UDatasmithScene>(AssetObject))
        {
            if (!DatasmithScene)
            {
                UE_LOG(LogTemp, Warning, TEXT("Datasmith Scene is invalid."));
                return;
            }

            ADatasmithSceneActor* RootActor = nullptr;
            ULevel* InLevel = GWorld->GetCurrentLevel();

            FTransform Transform = FTransform::Identity;

            EObjectFlags Flags = EObjectFlags::RF_Transactional;

            // Obter o mundo do editor
            UWorld* World = GEditor->GetEditorWorldContext().World();
            if (!World)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to get Editor World."));
                return;
            }

            if (!DatasmithScene || !DatasmithScene->AssetImportData)
            {
                return; // Can't import without the AssetImportData
            }

            TSharedPtr< IDatasmithScene > DatasmithSceneElement = FDatasmithImporterUtils::LoadDatasmithScene(DatasmithScene);

            if (!DatasmithSceneElement.IsValid())
            {
                return;
            }


            FName LoggerName = TEXT("DatasmithActorFactory");
            FText LoggerLabel = FText::FromString(TEXT("Datasmith Scene"));

            TSharedPtr<UE::DatasmithImporter::FExternalSource> ExternalSource;
            if (TObjectPtr<UDatasmithSceneImportData> DatasmithImportData = Cast<UDatasmithSceneImportData>(DatasmithScene->AssetImportData))
            {
                ExternalSource = IExternalSourceModule::Get().GetManager().TryGetExternalSourceFromImportData(*DatasmithImportData.Get());
            }

            const FString ImportPath = DatasmithScene->AssetImportData->BaseOptions.AssetOptions.PackagePath.ToString();
            FDatasmithImportContext ImportContext(ExternalSource, false, LoggerName, LoggerLabel);

            ImportContext.Options->BaseOptions = DatasmithScene->AssetImportData->BaseOptions;
            ImportContext.Options->BaseOptions.SceneHandling = EDatasmithImportScene::CurrentLevel;

            if (InLevel)
            {
                ImportContext.ActorsContext.ImportWorld = InLevel->GetWorld();
            }

            const bool bSilent = true;
            TSharedPtr<FJsonObject> JsonOptions;
            ImportContext.InitScene(DatasmithSceneElement.ToSharedRef());
            if (!ImportContext.Init(ImportPath, Flags, GWarn, JsonOptions, bSilent))
            {
                return;
            }

            if (!RootActor)
            {
                RootActor = FDatasmithImporterUtils::CreateImportSceneActor(ImportContext, Transform);
            }

            if (!RootActor)
            {
                return;
            }

            ImportContext.ActorsContext.FinalSceneActors.Add(RootActor);
            ImportContext.bIsAReimport = true;
            ImportContext.Options->ReimportOptions.bRespawnDeletedActors = false;


            ImportContext.SceneAsset = DatasmithScene;

            // The actor might get deleted or become unreachable if the user cancel the import/finalize
            TWeakObjectPtr<ADatasmithSceneActor> RootObjectAsWeakPtr = RootActor;

            FDatasmithImporter::ImportActors(ImportContext);
            FDatasmithImporter::FinalizeActors(ImportContext, nullptr);

            RootActor = RootObjectAsWeakPtr.Get();

            if (RootActor)
            {
                // If the root actor is still valid, ensure that it is in a world.
                if (!RootActor->GetWorld())
                {
                    RootActor = nullptr;
                }
            }
            HarvestComponentsAndCreateBlueprint(RootActor, APuzzleModel::StaticClass(), AlphaCurve);

            for (auto& Pair : RootActor->RelatedActors)
            {
                if (AActor* ChildActor = Pair.Value.Get())
                {
                    if (ChildActor->IsValidLowLevel())
                    {
                        UE_LOG(LogTemp, Log, TEXT("Deleting child actor: %s"), *ChildActor->GetName());
                        ChildActor->Destroy(); // Deletar o ator filho
                    }
                }
            }

            if (RootActor->IsValidLowLevel())
            {
                UE_LOG(LogTemp, Log, TEXT("Deleting SceneActor: %s"), *RootActor->GetName());
                RootActor->Destroy(); // Deletar o ator raiz
            }

        }
    }
    GEditor->RedrawAllViewports();

}

UCurveFloat* UAAU_UpdateModelMeshes::LoadCurveFromPath(const FString& CurvePath)
{
    // Usando StaticLoadObject para carregar o asset
    UCurveFloat* LoadedCurve = Cast<UCurveFloat>(StaticLoadObject(UCurveFloat::StaticClass(), nullptr, *CurvePath));

    if (LoadedCurve)
    {
        UE_LOG(LogTemp, Log, TEXT("Curve loaded successfully: %s"), *LoadedCurve->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load curve from path: %s"), *CurvePath);
    }

    return LoadedCurve;
}


void UAAU_UpdateModelMeshes::HarvestComponentsAndCreateBlueprint(ADatasmithSceneActor* SceneActor, UClass* ParentClass, UCurveFloat* AlphaCurve)
{
    if (!SceneActor || !ParentClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SceneActor or ParentClass is null."));
        return;
    }

    // Criar a nova Blueprint child
    FString BPName = FString::Printf(TEXT("BP_%s"), *SceneActor->GetActorLabel());
    UPackage* Package = CreatePackage(*FString::Printf(TEXT("/Game/Blueprints/Sculptures/%s"), *BPName));

    UBlueprint* NewBlueprint = FKismetEditorUtilities::CreateBlueprint(
        ParentClass, Package, *BPName, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());

    if (!NewBlueprint)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create new Blueprint."));
        return;
    }

    // Obter o SimpleConstructionScript do blueprint
    USimpleConstructionScript* SCS = NewBlueprint->SimpleConstructionScript;
    if (!SCS)
    {
        UE_LOG(LogTemp, Error, TEXT("SCS is invalid."));
        return;
    }

    // Modificar o Blueprint
    NewBlueprint->Modify();
    
    //Adding rootNode
    USCS_Node* RootNode = SCS->CreateNode(USceneComponent::StaticClass(), TEXT("Root"));
    SCS->AddNode(RootNode);


    FString CompletedModelName = FString::Printf(TEXT("CompletedModel"));
    USCS_Node* CompletedModelNode = SCS->CreateNode(UStaticMeshComponent::StaticClass(), *CompletedModelName);
    UStaticMeshComponent* CompletedModelMesh = Cast<UStaticMeshComponent>(CompletedModelNode->ComponentTemplate);
    CompletedModelMesh->bVisibleInSceneCaptureOnly = true;
    SCS->AddNode(CompletedModelNode);


    // Validar o nó raiz padrão
    SCS->ValidateSceneRootNodes();

    // Guardar novos nós criados
    TArray<USCS_Node*> NewNodes;

    int32 count = 0;
    int32 innerMeshCount = 0;

    for (auto& Pair : SceneActor->RelatedActors)
    {
        if (AActor* Actor = Pair.Value.Get())
        {
            if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor))
            {
                UStaticMeshComponent* StaticMeshComponent = StaticMeshActor->GetStaticMeshComponent();
                auto label = StaticMeshComponent->GetOwner()->GetActorLabel();
                
                auto name = StaticMeshComponent->GetOwner()->GetName();

                if (label.Contains(TEXT("lookat_sphere")))
                {
                    FVector Location = StaticMeshComponent->GetRelativeLocation();
                    FRotator Rotation = StaticMeshComponent->GetRelativeRotation();
                    FVector Scale = StaticMeshComponent->GetRelativeScale3D();

                    FString NewName = FString::Printf(TEXT("Lookat_sphere_%d"), innerMeshCount++);
                    USCS_Node* NewNode = SCS->CreateNode(UInnerMesh::StaticClass(), *NewName);

                    UInnerMesh* NewInnerMesh = Cast<UInnerMesh>(NewNode->ComponentTemplate);

                    if (!NewInnerMesh)
                    {
                        UE_LOG(LogTemp, Error, TEXT("Failed to create NewInnerMesh for %s"), *StaticMeshComponent->GetName());
                        continue;
                    }

                    NewInnerMesh->SetRelativeLocation(Location);
                    NewInnerMesh->SetRelativeRotation(Rotation);
                    NewInnerMesh->SetRelativeScale3D(Scale);
                    NewNodes.Add(NewNode);

                }
                else
                {
                    bool IsShell = label.Contains(TEXT("shell"));
                    if (IsShell)
                    {
                        // Salve as propriedades da Shell
                        UStaticMesh* Shell = StaticMeshComponent->GetStaticMesh();
                        FVector ShellLocation = StaticMeshComponent->GetRelativeLocation();
                        FRotator ShellRotation = StaticMeshComponent->GetRelativeRotation();
                        FVector ShellScale = StaticMeshComponent->GetRelativeScale3D();

                        FString NewShellParentName = FString::Printf(TEXT("PuzzlePart_Shell_Parent"));


                        USCS_Node* NewShellParentNode = SCS->CreateNode(UPuzzlePieceParentComponent::StaticClass(), *NewShellParentName);
                        UPuzzlePieceParentComponent* NewShellParentComponent = Cast<UPuzzlePieceParentComponent>(NewShellParentNode->ComponentTemplate);

                        if (NewShellParentComponent)
                        {
                            // Copie as propriedades de transformação
                            NewShellParentComponent->SetRelativeLocation(ShellLocation);
                            NewShellParentComponent->SetWorldRotation(ShellRotation);
                            NewShellParentComponent->SetWorldScale3D(FVector::One());
                            NewShellParentComponent->SetIsShell(true);


                            // Criar um novo nó com UPuzzlePiecesComponent para a Shell
                            FString NewChildName = FString::Printf(TEXT("PuzzlePart_Shell"));
                            USCS_Node* NewShellNode = SCS->CreateNode(UPuzzlePiecesComponent::StaticClass(), *NewChildName);
                            UPuzzlePiecesComponent* NewPuzzlePartComponent = Cast<UPuzzlePiecesComponent>(NewShellNode->ComponentTemplate);

                            if (NewPuzzlePartComponent)
                            {
                                // Copie as propriedades
                                NewPuzzlePartComponent->SetStaticMesh(Shell);

                                // Definir o perfil de colisão padrão
                                NewPuzzlePartComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));

                                NewPuzzlePartComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore); // Supondo que Pieces é ECC_GameTraceChannel1
                                NewPuzzlePartComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Block); // Supondo que Shell é ECC_GameTraceChannel2
                                NewPuzzlePartComponent->bHiddenInSceneCapture = true;


                                // Verificar antes de anexar o novo nó de componente de cena ao novo nó de PuzzlePart
                                if (ensure(NewShellNode) && ensure(NewShellParentNode))
                                {
                                    NewShellParentNode->AddChildNode(NewShellNode);
                                }

                            }

                            NewNodes.Add(NewShellParentNode);
                        }

 
                    }
                    else if(label.Contains(TEXT("parent_plane")))
                    {
                        // Salve as propriedades de transformação
                        FVector Location = StaticMeshComponent->GetRelativeLocation();
                        FRotator Rotation = StaticMeshComponent->GetRelativeRotation();
                        FVector OriginalScale = StaticMeshComponent->GetComponentScale();


                        FVector OppositeDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Z);
                        FRotator OppositeRotator = UKismetMathLibrary::FindLookAtRotation(Location, Location + OppositeDirection);


                        // Criar um novo nó de componente de cena
                        FString NewComponentName = FString::Printf(TEXT("PuzzlePart_%d_Parent"), count);

                        USCS_Node* NewSceneNode = SCS->CreateNode(UPuzzlePieceParentComponent::StaticClass(), *NewComponentName);
                        UPuzzlePieceParentComponent* NewSceneComponent = Cast<UPuzzlePieceParentComponent>(NewSceneNode->ComponentTemplate);

                        if (NewSceneComponent)
                        {
                            // Copie as propriedades de transformação
                            NewSceneComponent->SetRelativeLocation(Location);
                            NewSceneComponent->SetWorldRotation(OppositeRotator);
                            NewSceneComponent->SetWorldScale3D(FVector::One());
                            NewSceneComponent->SetIsShell(false);
                            NewSceneComponent->SetLerpCurve(AlphaCurve);

                            // Realocar filhos para o novo nó de componente de cena

                            TArray<USceneComponent*> Childs;

                            StaticMeshComponent->GetChildrenComponents(true, Childs);
                            for (auto ChildNode : Childs)
                            {
                                UStaticMeshComponent* ChildStaticMeshComponent = Cast<UStaticMeshComponent>(ChildNode);
                                if (ChildStaticMeshComponent)
                                {
                                    // Salve as propriedades da child
                                    UStaticMesh* Mesh = ChildStaticMeshComponent->GetStaticMesh();
                                    FVector ChildLocation = ChildStaticMeshComponent->GetRelativeLocation();
                                    FRotator ChildRotation = ChildStaticMeshComponent->GetComponentRotation();
                                    FVector ChildScales = ChildStaticMeshComponent->GetRelativeScale3D();

                                    // Criar um novo nó com UPuzzlePiecesComponent para a child
                                    FString NewChildName = FString::Printf(TEXT("PuzzlePart_%d"), count);
                                    USCS_Node* NewChildNode = SCS->CreateNode(UPuzzlePiecesComponent::StaticClass(), *NewChildName);
                                    UPuzzlePiecesComponent* NewPuzzlePartComponent = Cast<UPuzzlePiecesComponent>(NewChildNode->ComponentTemplate);

                                    if (NewPuzzlePartComponent)
                                    {
                                        // Copie as propriedades
                                        NewPuzzlePartComponent->SetStaticMesh(Mesh);
                                        NewPuzzlePartComponent->SetRelativeLocation(ChildLocation);


                                        FRotator PieceEndRotator2 = UKismetMathLibrary::ComposeRotators(ChildRotation, UKismetMathLibrary::NegateRotator(OppositeRotator));
                                        NewPuzzlePartComponent->SetWorldRotation(PieceEndRotator2);
                                        NewPuzzlePartComponent->SetWorldScale3D(ChildScales * OriginalScale);
                                        //NewPuzzlePartComponent->SetRelativeLocation(FVector::Zero());

                                        // Definir o perfil de colisão padrão
                                        NewPuzzlePartComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));

                                        // Ajustar canais específicos
                                        NewPuzzlePartComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Block); // Supondo que Pieces é ECC_GameTraceChannel1
                                        NewPuzzlePartComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore); // Supondo que Shell é ECC_GameTraceChannel2
                                        NewPuzzlePartComponent->bHiddenInSceneCapture = true;

                                        // Verificar antes de anexar o novo nó de componente de cena ao novo nó de PuzzlePart
                                        if (ensure(NewSceneNode) && ensure(NewChildNode))
                                        {
                                            NewSceneNode->AddChildNode(NewChildNode);
                                        }
                                    }
                                }
                            }

                            // Adicionar o novo nó à lista temporária
                            NewNodes.Add(NewSceneNode);
                        }
                    }
                    count++;
                }
            }
        }

    }

    // Adicionar os novos nós à Blueprint
    for (USCS_Node* NewNode : NewNodes)
    {
        SCS->AddNode(NewNode);
    }

    // Compilar e salvar a Blueprint
    FKismetEditorUtilities::CompileBlueprint(NewBlueprint);
    FBlueprintEditorUtils::MarkBlueprintAsModified(NewBlueprint);
    FAssetRegistryModule::AssetCreated(NewBlueprint);
    Package->MarkPackageDirty();

    FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());

    // Salvar o pacote
    bool bSuccess = UPackage::SavePackage(
        Package,
        NewBlueprint,
        EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
        *PackageFileName,
        GError,
        nullptr,
        true,
        true,
        SAVE_None);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Blueprint '%s' compilada e salva com sucesso!"), *NewBlueprint->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Erro ao salvar a Blueprint '%s'."), *NewBlueprint->GetName());
    }

    UE_LOG(LogTemp, Log, TEXT("Blueprint '%s' created and compiled successfully!"), *BPName);
}

void UAAU_UpdateModelMeshes::SetAllStaticMeshesHiddenInSceneCapture()
{
    // Obtém o mundo atual do editor
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return;

    // Itera sobre todos os atores StaticMesh na cena
    for (FActorIterator ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            // Obtém todos os componentes StaticMesh
            TArray<UStaticMeshComponent*> StaticMeshComponents;
            Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);

            // Itera por todos os componentes StaticMesh encontrados
            for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
            {
                if (StaticMeshComponent)
                {
                    // Define a propriedade HiddenInSceneCapture para true
                    StaticMeshComponent->bHiddenInSceneCapture = true;

                    // Marca o componente como modificado para garantir que o editor reconheça a mudança
                    StaticMeshComponent->Modify();
                    StaticMeshComponent->MarkRenderStateDirty(); // Marca o componente para ser atualizado
                }
            }

            // Marca o ator como modificado após atualizar todos os componentes
            Actor->Modify();
        }
    }

    // Exibe uma mensagem no log para confirmar que a operação foi realizada
    UE_LOG(LogTemp, Log, TEXT("Todos os StaticMeshActors foram configurados para Hidden in Scene Capture."));
}

#endif