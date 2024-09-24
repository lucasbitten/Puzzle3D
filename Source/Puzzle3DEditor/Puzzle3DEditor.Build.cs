// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Puzzle3DEditor : ModuleRules
{
    public Puzzle3DEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "EditorStyle",
            "UnrealEd",
            "Blutility", // Para utilitários de script do editor
            "EditorScriptingUtilities",
            "DatasmithContent",
            "DatasmithCore",
            "DatasmithImporter",
            "ExternalSource"
        });

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Puzzle3D" // Adicione seu módulo de jogo aqui
        });
    }
}
