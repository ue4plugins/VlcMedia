// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class VlcMediaEditor : ModuleRules
	{
		public VlcMediaEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
					"MediaAssets",
					"UnrealEd",
                }
			);

			PrivateIncludePaths.AddRange(
				new string[] {
					"VlcMediaEditor/Private",
				}
			);
		}
	}
}
