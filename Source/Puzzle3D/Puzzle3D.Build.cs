// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Puzzle3D : ModuleRules
{
	public Puzzle3D(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore"});

		PrivateDependencyModuleNames.AddRange(new string[] {
            "Slate",
            "SlateCore",
            "EditorScriptingUtilities", // Adicione este m�dulo
            "UnrealEd", // Adicione este m�dulo
            "Blutility", // Adicione este m�dulo
            "AssetTools", // Adicione este m�dulo
            "AssetRegistry" // Adicione este m�dulo
		
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
