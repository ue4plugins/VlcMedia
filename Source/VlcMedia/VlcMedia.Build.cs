// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.
// Modified by @anselm

namespace UnrealBuildTool.Rules
{
	public class VlcMedia : ModuleRules
	{
		public VlcMedia(TargetInfo Target)
		{
			DynamicallyLoadedModuleNames.AddRange(
				new string[] {
					"Media", "Settings",
				}
			);

			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"Core","CoreUObject","Engine", "RenderCore",
				}
			);

			PrivateIncludePathModuleNames.AddRange(
				new string[] {
					"Media", "Settings",
				}
			);

			PrivateIncludePaths.AddRange(
				new string[] {
					"VlcMedia/Private", "VlcMedia/Private/Player",
				}
			);


			if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
			{
				//PublicDelayLoadDLLs.Add("shlwapi.dll");
			}

			if (Target.Platform == UnrealTargetPlatform.Linux)
			{
				AddThirdPartyPrivateStaticDependencies(Target, "SDL2");
				PublicAdditionalLibraries.Add("/usr/lib/libvlc.so");
			}
		}
	}
}
