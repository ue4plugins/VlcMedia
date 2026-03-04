# VlcMedia

Unreal Engine Media Framework plugin using VideoLAN libvlc.

## About

This repository is based on the original `ue4plugins/VlcMedia` project and has
been updated to run on Unreal Engine 5.6.

The plugin is still community maintained. Test carefully before production use.

## Engine and Platform Status

- Engine target: **Unreal Engine 5.6** (`VlcMedia.uplugin`)
- Confirmed working path in this branch: **Win64**
- Linux and Mac modules are still present, but may require platform-specific
  validation and packaging updates.

## Licensing and Distribution Notes

This repository includes precompiled libvlc binaries and plugins licensed under
LGPL.

Important requirements:

- Keep libvlc as dynamically loaded libraries (DLL/dylib/so)
- Do not statically link libvlc into game binaries
- Keep licensing and notice files in `ThirdParty/vlc`

Because of these constraints, platforms without practical dynamic library plugin
support are not expected to work.

## Prerequisites

- Unreal Engine 5.6
- Visual Studio toolchain for C++ builds on Windows
- A C++ project (or full source engine build) to compile plugin modules

## Included Runtime Binaries

`ThirdParty/vlc` contains bundled VLC runtime files used by the plugin.

- Win64 binaries and plugins are refreshed in this branch
- Linux helper scripts are available in `Build/` for local libvlc setup

If you need to swap libvlc builds, replace the corresponding files under
`ThirdParty/vlc/<Platform>` and re-test playback.

## Installation

### As Project Plugin (recommended)

1. Copy or clone this repository into your project at `Plugins/VlcMedia`
2. Regenerate project files if needed
3. Build the project from Visual Studio (or via Unreal Build Tool)

### As Engine Plugin

1. Place repository under `Engine/Plugins/Media/VlcMedia`
2. Rebuild engine/project modules

## RTSP Notes

RTSP-specific setup notes are available in `RTSP_SETUP.md`.

### UE5.6 RTSP Preview

The screenshot below shows RTSP playback running in UE 5.6 with this plugin setup.

![UE5.6 RTSP CCTV preview](Resources/images/ue56-rtsp-cctv-preview.png)

## References

- [VideoLAN](https://www.videolan.org/)
- [VlcMedia upstream repository](https://github.com/ue4plugins/VlcMedia)
