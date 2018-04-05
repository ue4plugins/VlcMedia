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
				VlcDirectory = Path.Combine(VlcDirectory, Target.Architecture, "lib");
				RuntimeDependencies.Add(Path.Combine(VlcDirectory, "libvlc.so"));
				RuntimeDependencies.Add(Path.Combine(VlcDirectory, "libvlc.so.5"));
				RuntimeDependencies.Add(Path.Combine(VlcDirectory, "libvlc.so.5.6.0"));
				RuntimeDependencies.Add(Path.Combine(VlcDirectory, "libvlccore.so"));
				RuntimeDependencies.Add(Path.Combine(VlcDirectory, "libvlccore.so.9"));
				RuntimeDependencies.Add(Path.Combine(VlcDirectory, "libvlccore.so.9.0.0"));
			}
			else if (Target.Platform == UnrealTargetPlatform.Mac)
			{
				RuntimeDependencies.Add(Path.Combine(VlcDirectory, "libvlc.dylib"));
				RuntimeDependencies.Add(Path.Combine(VlcDirectory, "libvlc.5.dylib"));
				RuntimeDependencies.Add(Path.Combine(VlcDirectory, "libvlccore.dylib"));
				RuntimeDependencies.Add(Path.Combine(VlcDirectory, "libvlccore.9.dylib"));
			}
			else if (Target.Platform == UnrealTargetPlatform.Win32)
			{
				RuntimeDependencies.Add(Path.Combine(VlcDirectory, "libvlc.dll"));
				RuntimeDependencies.Add(Path.Combine(VlcDirectory, "libvlccore.dll"));
			}
			else if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				RuntimeDependencies.Add(Path.Combine(VlcDirectory, "libvlc.dll"));
				RuntimeDependencies.Add(Path.Combine(VlcDirectory, "libvlccore.dll"));
			}

			// add VLC plug-ins
			string PluginDirectory = Path.Combine(VlcDirectory, "plugins");
            
			if (Target.Platform == UnrealTargetPlatform.Linux)
			{
				PluginDirectory = Path.Combine(VlcDirectory, "vlc", "plugins");
			}

			if (Directory.Exists(PluginDirectory))
			{
				foreach (string Plugin in Directory.EnumerateFiles(PluginDirectory, "*.*", SearchOption.AllDirectories))
				{
					RuntimeDependencies.Add(Path.Combine(PluginDirectory, Plugin));
				}
			}
		}
	}
}
