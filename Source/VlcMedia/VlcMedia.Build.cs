// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class VlcMedia : ModuleRules
	{
		public VlcMedia(TargetInfo Target)
		{
            DynamicallyLoadedModuleNames.AddRange(
                new string[] {
                    "Media",
				}
            );

			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"Core",
                    "RenderCore",
				}
			);

			PrivateIncludePathModuleNames.AddRange(
				new string[] {
                    "Media",
				}
			);

			PrivateIncludePaths.AddRange(
				new string[] {
					"VlcMedia/Private",
                    "VlcMedia/Private/Player",
				}
			);

            if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
            {
                //PublicDelayLoadDLLs.Add("shlwapi.dll");
            }
		}
	}
}
