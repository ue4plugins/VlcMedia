# VlcMedia

Unreal Engine 4 Media Framework plug-in using the Video LAN Codec (libvlc).


## About

This plug-in is still under development and likely has a lot of remaining issues to be fixed. Use in production is not yet recommended.


## Supported Platforms

The following platforms have been built and tested:

- ~~Linux~~
- ~~Mac~~
- Windows

**IMPORTANT**: Please note that this repository contains pre-compiled binaries for libvlc and its plug-ins, which are licensed under LGPL. This means that you cannot create monolithic builds of your game without violating LGPL, the UE4 EULA or both. The libvlc libraries must remain dynamic libraries that are bound at run-time - static linking is not allowed.

This also means that this plug-in cannot work on platforms that do not support dynamically linked libraries (i.e. iOS, HTML5) or do not currently implement support for it (i.e. Android, XboxOne).


## Prerequisites

A relatively recent version of libvlc is required. The latest stable release (currently 2.2.1) is not sufficient.

For Mac and Windows, the following nightly builds are currently included:
* Mac: vlc-3.0.0-x86_64-20150618-0108
* Win32: vlc-3.0.0-git-20150605-0002-win32
* Win64: vlc-3.0.0-git-20150605-0403-win64

Nightly builds can be downloaded from the VideoLAN web site (see below).

### Linux

A suitable version of **libvlc** must be installed or compiled from source.

### Mac, Windows

The libvlc libraries and plug-ins are already included in the plug-ins *ThirdParty* directory and will work out of the box.


## Dependencies

This plug-in requires Unreal Engine 4 source code from the Master branch.


## Usage

Clone this repository into your game's */Engine/Plugins/Media* directory and compile your game.


## Support

Please [file an issue](https://github.com/ue4plugins/VlcMedia/issues), submit a
[pull request](https://github.com/ue4plugins/VlcMedia/pulls?q=is%3Aopen+is%3Apr)
or email us at info@headcrash.industries.


## References

* [VideoLAN Homepage](http://videolan.org)
* [VideoLAN Nightly Builds](http://nightlies.videolan.org/)
