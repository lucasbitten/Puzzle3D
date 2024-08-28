#include "Puzzle3DEditor.h"
#include "Modules/ModuleManager.h"

void FPuzzle3DEditorModule::StartupModule()
{
    UE_LOG(LogTemp, Warning, TEXT("Puzzle3DEditor module has started!"));
    // Seu código de inicialização
}

void FPuzzle3DEditorModule::ShutdownModule()
{
    UE_LOG(LogTemp, Warning, TEXT("Puzzle3DEditor module is shutting down!"));
    // Seu código de limpeza
}

IMPLEMENT_MODULE(FPuzzle3DEditorModule, Puzzle3DEditor)