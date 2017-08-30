// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
	using System.IO;

	public class VlcMedia : ModuleRules
	{
		public VlcMedia(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			DynamicallyLoadedModuleNames.AddRange(
				new string[] {
					"Media",
				});

			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
					"MediaUtils",
					"Projects",
					"RenderCore",
					"VlcMediaFactory",
				});

			PrivateIncludePathModuleNames.AddRange(
				new string[] {
					"Media",
				});

			PrivateIncludePaths.AddRange(
				new string[] {
					"VlcMedia/Private",
					"VlcMedia/Private/Player",
					"VlcMedia/Private/Shared",
					"VlcMedia/Private/Vlc",
				});

			// add VLC libraries
			string BaseDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
			string VlcDirectory = Path.Combine(BaseDirectory, "ThirdParty", "vlc", Target.Platform.ToString());

			if (Target.Platform == UnrealTargetPlatform.Linux)
			{
				VlcDirectory = Path.Combine(VlcDirectory, Target.Architecture);

				RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlc.so.5.5.0")));
				RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlccore.so.8.0.0")));
				RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlc.so.5")));
				RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlccore.so.8")));
				RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlc.so")));
				RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlccore.so")));
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
				RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlc.dll")));
				RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlccore.dll")));
			}
			else if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlc.dll")));
				RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(VlcDirectory, "libvlccore.dll")));
			}

			// add VLC plug-ins
			string PluginDirectory = Path.Combine(VlcDirectory, "plugins");

			if (Directory.Exists(PluginDirectory))
			{
				foreach (string Plugin in Directory.EnumerateFiles(PluginDirectory, "*.*", SearchOption.AllDirectories))
				{
					RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(PluginDirectory, Plugin)));
				}
			}
		}
	}
}
