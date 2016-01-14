# VlcMedia

Unreal Engine 4 Media Framework plug-in using the Video LAN Codec (libvlc).


## About

This plug-in is still under development and likely has a lot of remaining issues to be fixed. Use in production is not yet recommended.


## Supported Platforms

This plug-in was last built against **Unreal Engine 4.11 Preview 1** and tested
against the following platforms:

- ~~Linux~~
- ~~Mac~~
- Windows

**IMPORTANT**: Please note that this repository contains pre-compiled binaries for libvlc and its plug-ins, which are licensed under LGPL. This means that you cannot create monolithic builds of your game without violating LGPL, the UE4 EULA or both. The libvlc libraries must remain dynamic libraries that are bound at run-time - static linking is not allowed - and the licensing related files in */ThirdParty/vlc* must be retained.

This also means that this plug-in cannot work on platforms that do not support dynamically linked libraries (i.e. iOS, HTML5) or do not currently implement support for it (i.e. Android, XboxOne).

Epic is in the process of adding plug-in support to monolithic builds, but there is no ETA yet. Once this is supported, you will be able to distribute monolithic game and server builds with VlcMedia, provided that the libvlc libraries and plug-ins remain as separate DLLs.


## Prerequisites

A relatively recent version of libvlc is required. The latest stable release (currently 2.2.1) is not sufficient.

For Mac and Windows, the following nightly builds are currently included:
* Mac: vlc-3.0.0-x86_64-20160113-0109
* Win32: vlc-3.0.0-git-20151221-0002-win32
* Win64: vlc-3.0.0-git-20151220-0402-win64

Nightly builds can be downloaded from the VideoLAN web site (see below).

### Linux

A suitable version of **libvlc** must be installed or compiled from source. If you ship your game on  Linux, you will likely want to include libvlc with it, so that users don't have to install it themselves. We will eventually include those binaries in this repository, although it is not clear what distros should be supported and where the builds are coming from. A better workflow needs to be established for this.

### Mac, Windows

All required libraries and plug-ins are included in the *ThirdParty* directory and work out of the box.


## Dependencies

This plug-in requires Unreal Engine 4 source code from the Master branch.


## Usage

Clone this repository into your game's */Engine/Plugins/Media* directory and compile your game.


## Support

Please [file an issue](https://github.com/ue4plugins/VlcMedia/issues), submit a
[pull request](https://github.com/ue4plugins/VlcMedia/pulls?q=is%3Aopen+is%3Apr)
or email us at info@headcrash.industries


## References

* [VideoLAN Homepage](http://videolan.org)
* [VideoLAN Nightly Builds](http://nightlies.videolan.org/)
