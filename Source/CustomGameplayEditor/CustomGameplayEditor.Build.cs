// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CustomGameplayEditor : ModuleRules
{
	public CustomGameplayEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;
		
		// --- 런타임에 필요한 기본 모듈 ---
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"AnimGraphRuntime",
				"AnimationCore",
				"KillingFloorLike" // 프로젝트 런타임 모듈
			}
		);
        
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore"
			}
		);
        
		// --- 에디터 빌드에서만 필요한 모듈 ---
		// 참고: 이 모듈 자체가 Editor 타입이라 사실 이 if문은 필수는 아니지만,
		//      런타임 모듈과의 일관성을 위해 추가하는 것도 좋은 방법입니다.
		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
					"AssetTools",
					"AssetRegistry",
					"EditorScriptingUtilities",
					"AnimGraph",
					"AnimationEditor",
					"BlueprintGraph"
				}
			);
		}
	}
}