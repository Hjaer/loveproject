// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Gercek : ModuleRules
{
	public Gercek(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "PCG", "UMG", "OnlineSubsystem", "OnlineSubsystemSteam", "OnlineSubsystemUtils", "DeveloperSettings" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
	}
}
