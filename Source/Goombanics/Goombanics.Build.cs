// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class Goombanics : ModuleRules
{
	public Goombanics(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// NOTE (WRITE-ONLY / Windows build later):
		// This project was authored with feature folders under Source/Goombanics/*.
		// We do NOT move files on macOS. Instead we explicitly add include paths so
		// Windows builds (VS2022) will compile cleanly without relying on brittle relative includes.
		//
		// TODO(Phase2-Windows): Consider migrating source layout to strict Public/Private with
		// feature subfolders under each, once the project compiles end-to-end.
		PublicIncludePaths.AddRange(new string[]
		{
			Path.Combine(ModuleDirectory, "Public"),
			ModuleDirectory,
			Path.Combine(ModuleDirectory, "Core"),
			Path.Combine(ModuleDirectory, "Player"),
			Path.Combine(ModuleDirectory, "Monster"),
			Path.Combine(ModuleDirectory, "Weapons"),
			Path.Combine(ModuleDirectory, "Destruction"),
			Path.Combine(ModuleDirectory, "UI"),
		});

		PrivateIncludePaths.AddRange(new string[]
		{
			Path.Combine(ModuleDirectory, "Private"),
		});

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"GameplayTags",
			"UMG",
			"Slate",
			"SlateCore",
			"NetCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { 
			"NavigationSystem"
		});
	}
}
