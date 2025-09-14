// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class KillingFloorLike : ModuleRules
{
    public KillingFloorLike(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // --- 런타임에 필요한 모듈 ---
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput", "GameplayTasks",
            "Niagara", "NavigationSystem",
            "AIModule",
            "GameplayTags",
            "GameplayMessageRuntime",
            "OnlineSubsystem", "OnlineSubsystemNull", "OnlineSubsystemSteam",
            "Slate", "SlateCore", "AdvancedSessions"
        });
        
        // --- 에디터 빌드에서만 필요한 모듈 ---
        if (Target.bBuildEditor)
        {
            PublicDependencyModuleNames.AddRange(new string[] 
            { 
                "UnrealEd", 
                "AssetTools",
                "Blutility"
            });
        }
    }
}