// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class VlcMediaFactories : ModuleRules
	{
		public VlcMediaFactories(TargetInfo Target)
		{
			DynamicallyLoadedModuleNames.AddRange(
				new string[] {
					"Media",
				}
			);

			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
					"MediaAssets",
				}
			);

			PrivateIncludePathModuleNames.AddRange(
				new string[] {
					"Media",
					"VlcMedia",
				}
			);

			PrivateIncludePaths.AddRange(
				new string[] {
					"VlcMediaFactories/Private",
				}
			);
			
			if (UEBuildConfiguration.bBuildEditor)
			{
				PrivateDependencyModuleNames.Add("UnrealEd");
			}

			if ((Target.Platform == UnrealTargetPlatform.Mac) ||
				(Target.Platform == UnrealTargetPlatform.Linux) ||
				(Target.Platform == UnrealTargetPlatform.Win32) ||
				(Target.Platform == UnrealTargetPlatform.Win64) ||
				(Target.Platform == UnrealTargetPlatform.WinRT))
				{
					DynamicallyLoadedModuleNames.Add("VlcMedia");
				}
		}
	}
}
