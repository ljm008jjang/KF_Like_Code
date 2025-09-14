// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class KillingFloorLikeServerTarget : TargetRules
{
	public KillingFloorLikeServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		ExtraModuleNames.Add("KillingFloorLike"); // 프로젝트 주 모듈 이름
	}
}