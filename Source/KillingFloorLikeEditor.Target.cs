// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class KillingFloorLikeEditorTarget : TargetRules
{
	public KillingFloorLikeEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("KillingFloorLike");
		RegisterModulesCreatedByRider();
	}

	private void RegisterModulesCreatedByRider()
	{
		ExtraModuleNames.AddRange(new string[] { "CustomGameplay", "CustomGameplayEditor" });
	}
}
