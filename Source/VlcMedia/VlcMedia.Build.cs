// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
    using System.IO;

	public class VlcMedia : ModuleRules
	{
		public VlcMedia(TargetInfo Target)
		{         
            DynamicallyLoadedModuleNames.AddRange(
				new string[] {
					"Media",
                    "Settings",
				}
			);

			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"Core",
                    "CoreUObject",
                    "Projects",
                    "RenderCore",
				}
			);

			PrivateIncludePathModuleNames.AddRange(
				new string[] {
					"Media",
                    "Settings",
				}
			);

			PrivateIncludePaths.AddRange(
				new string[] {
					"VlcMedia/Private",
                    "VlcMedia/Private/Player",
                    "VlcMedia/Private/Shared",
                    "VlcMedia/Private/Tracks",
                    "VlcMedia/Private/Vlc",
				}
			);

            // add VLC libraries
            string BaseDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
            string VlcDirectory = Path.Combine(BaseDirectory, "ThirdParty", "vlc", Target.Platform.ToString());

            if (Target.Platform == UnrealTargetPlatform.Linux)
            {
                // We expect libvlc to be installed for now until we can figure
                // out how to configure package dependencies at installation time.
            }
            else if (Target.Platform == UnrealTargetPlatform.Mac)
            {
                RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlc.dylib")));
                RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlc.5.dylib")));
                RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlccore.dylib")));
                RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlccore.8.dylib")));
            }
            else if (Target.Platform == UnrealTargetPlatform.Win32)
            {
                RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libgcc_s_sjlj-1.dll")));
                RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlc.dll")));
                RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlccore.dll")));
            }
            else if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libgcc_s_seh-1.dll")));
                RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlc.dll")));
                RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlccore.dll")));
            }
            else if (Target.Platform == UnrealTargetPlatform.WinRT)
            {
                RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlc.dll")));
                RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlccore.dll")));
            }

            // add VLC plug-ins
            string PluginDirectory = Path.Combine(VlcDirectory, "plugins");

            if (Directory.Exists(PluginDirectory))
            {
                foreach (string Plugin in Directory.EnumerateFiles(PluginDirectory))
                {
                    RuntimeDependencies.Add(new RuntimeDependency(Plugin));
                }
            }
		}
	}
}
