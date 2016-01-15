// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"
#include "IPluginManager.h"


#define VLC_IMPORT(Name, Func) \
	Func = (FLibvlc##Func##Proc)FPlatformProcess::GetDllExport(LibHandle, TEXT(#Name)); \
	if (Func == nullptr) \
	{ \
		UE_LOG(LogVlcMedia, Warning, TEXT("Failed to import VLC function %s"), TEXT(#Name)); \
		Shutdown(); \
		return false; \
	}

#define VLC_DEFINE(Func) \
	FLibvlc##Func##Proc FVlc::Func = nullptr


/* Static initialization
 *****************************************************************************/

void* FVlc::CoreHandle = nullptr;
void* FVlc::LibHandle = nullptr;
FString FVlc::PluginDir;

#if PLATFORM_WINDOWS
	void* FVlc::GccHandle = nullptr;
#endif



VLC_DEFINE(New);
VLC_DEFINE(Release);
VLC_DEFINE(Retain);

VLC_DEFINE(Errmsg);
VLC_DEFINE(Clearerr);

VLC_DEFINE(EventAttach);
VLC_DEFINE(EventDetach);
VLC_DEFINE(EventTypeName);

VLC_DEFINE(LogSet);
VLC_DEFINE(LogUnset);

VLC_DEFINE(MediaEventManager);
VLC_DEFINE(MediaGetStats);
VLC_DEFINE(MediaNewCallbacks);
VLC_DEFINE(MediaNewLocation);
VLC_DEFINE(MediaNewPath);
VLC_DEFINE(MediaParseAsync);
VLC_DEFINE(MediaRelease);
VLC_DEFINE(MediaRetain);
VLC_DEFINE(MediaTracksGet);
VLC_DEFINE(MediaTracksRelease);

VLC_DEFINE(MediaPlayerEventManager);
VLC_DEFINE(MediaPlayerGetMedia);
VLC_DEFINE(MediaPlayerNew);
VLC_DEFINE(MediaPlayerNewFromMedia);
VLC_DEFINE(MediaPlayerRelease);
VLC_DEFINE(MediaPlayerRetain);
VLC_DEFINE(MediaPlayerSetMedia);

VLC_DEFINE(MediaPlayerCanPause);
VLC_DEFINE(MediaPlayerGetFps);
VLC_DEFINE(MediaPlayerGetLength);
VLC_DEFINE(MediaPlayerGetPosition);
VLC_DEFINE(MediaPlayerGetRate);
VLC_DEFINE(MediaPlayerGetState);
VLC_DEFINE(MediaPlayerGetTime);
VLC_DEFINE(MediaPlayerIsSeekable);
VLC_DEFINE(MediaPlayerSetPosition);
VLC_DEFINE(MediaPlayerSetRate);
VLC_DEFINE(MediaPlayerSetTime);

VLC_DEFINE(MediaPlayerIsPlaying);
VLC_DEFINE(MediaPlayerPause);
VLC_DEFINE(MediaPlayerPlay);
VLC_DEFINE(MediaPlayerSetPause);
VLC_DEFINE(MediaPlayerStop);
VLC_DEFINE(MediaPlayerWillPlay);

VLC_DEFINE(AudioSetCallbacks);
VLC_DEFINE(AudioSetFormat);
VLC_DEFINE(AudioSetFormatCallbacks);
VLC_DEFINE(AudioGetTrack);
VLC_DEFINE(AudioSetTrack);

VLC_DEFINE(VideoGetHeight);
VLC_DEFINE(VideoGetWidth);
VLC_DEFINE(VideoSetCallbacks);
VLC_DEFINE(VideoSetFormat);
VLC_DEFINE(VideoSetFormatCallbacks);
VLC_DEFINE(VideoGetSpu);
VLC_DEFINE(VideoSetSpu);
VLC_DEFINE(VideoGetTrack);
VLC_DEFINE(VideoSetTrack);

VLC_DEFINE(AudioGetTrackDescription);
VLC_DEFINE(VideoGetSpuDescription);
VLC_DEFINE(VideoGetTrackDescription);
VLC_DEFINE(TrackDescriptionListRelease);


/* FVlc static functions
 *****************************************************************************/

FString FVlc::GetPluginDir()
{
	return PluginDir;
}


bool FVlc::Initialize()
{
	// determine directory paths
	const FString BaseDir = IPluginManager::Get().FindPlugin("VlcMedia")->GetBaseDir();
	const FString VlcDir = FPaths::Combine(*BaseDir, TEXT("ThirdParty"), TEXT("vlc"));

#if PLATFORM_LINUX
	const FString LibDir = FString();
#elif PLATFORM_MAC
	const FString LibDir = FPaths::Combine(*VlcDir, TEXT("Mac"));
#elif PLATFORM_WINDOWS
	#if PLATFORM_64BITS
		const FString LibDir = FPaths::Combine(*VlcDir, TEXT("Win64"));

		if (!LoadDependency(LibDir, TEXT("libgcc_s_seh-1"), GccHandle))
		{
			return false;
		}
	#else
		const FString LibDir = FPaths::Combine(*VlcDir, TEXT("Win32"));

		if (!LoadDependency(LibDir, TEXT("libgcc_s_sjlj-1"), GccHandle))
		{
			return false;
		}
	#endif
#endif

	// load required libraries in the correct order
	if (!LoadDependency(LibDir, TEXT("libvlccore"), CoreHandle) ||
		!LoadDependency(LibDir, TEXT("libvlc"), LibHandle))
	{
		Shutdown();
		return false;
	}

	// import library functions
	VLC_IMPORT(libvlc_new, New);
	VLC_IMPORT(libvlc_release, Release);
	VLC_IMPORT(libvlc_retain, Retain);

	VLC_IMPORT(libvlc_errmsg, Errmsg);
	VLC_IMPORT(libvlc_clearerr, Clearerr);

	VLC_IMPORT(libvlc_event_attach, EventAttach);
	VLC_IMPORT(libvlc_event_detach, EventDetach);
	VLC_IMPORT(libvlc_event_type_name, EventTypeName);

	VLC_IMPORT(libvlc_log_set, LogSet);
	VLC_IMPORT(libvlc_log_unset, LogUnset);

	VLC_IMPORT(libvlc_media_event_manager, MediaEventManager);
	VLC_IMPORT(libvlc_media_get_stats, MediaGetStats);
	VLC_IMPORT(libvlc_media_new_callbacks, MediaNewCallbacks);
	VLC_IMPORT(libvlc_media_new_location, MediaNewLocation);
	VLC_IMPORT(libvlc_media_new_path, MediaNewPath);
	VLC_IMPORT(libvlc_media_parse_async, MediaParseAsync);
	VLC_IMPORT(libvlc_media_release, MediaRelease);
	VLC_IMPORT(libvlc_media_retain, MediaRetain);
	VLC_IMPORT(libvlc_media_tracks_get, MediaTracksGet);
	VLC_IMPORT(libvlc_media_tracks_release, MediaTracksRelease);

	VLC_IMPORT(libvlc_media_player_event_manager, MediaPlayerEventManager);
	VLC_IMPORT(libvlc_media_player_get_media, MediaPlayerGetMedia);
	VLC_IMPORT(libvlc_media_player_new, MediaPlayerNew);
	VLC_IMPORT(libvlc_media_player_new_from_media, MediaPlayerNewFromMedia);
	VLC_IMPORT(libvlc_media_player_release, MediaPlayerRelease);
	VLC_IMPORT(libvlc_media_player_retain, MediaPlayerRetain);
	VLC_IMPORT(libvlc_media_player_set_media, MediaPlayerSetMedia);

	VLC_IMPORT(libvlc_media_player_can_pause, MediaPlayerCanPause);
	VLC_IMPORT(libvlc_media_player_get_fps, MediaPlayerGetFps);
	VLC_IMPORT(libvlc_media_player_get_length, MediaPlayerGetLength);
	VLC_IMPORT(libvlc_media_player_get_position, MediaPlayerGetPosition);
	VLC_IMPORT(libvlc_media_player_get_rate, MediaPlayerGetRate);
	VLC_IMPORT(libvlc_media_player_get_state, MediaPlayerGetState);
	VLC_IMPORT(libvlc_media_player_get_time, MediaPlayerGetTime);
	VLC_IMPORT(libvlc_media_player_is_seekable, MediaPlayerIsSeekable);
	VLC_IMPORT(libvlc_media_player_set_position, MediaPlayerSetPosition);
	VLC_IMPORT(libvlc_media_player_set_rate, MediaPlayerSetRate);
	VLC_IMPORT(libvlc_media_player_set_time, MediaPlayerSetTime);

	VLC_IMPORT(libvlc_media_player_is_playing, MediaPlayerIsPlaying);
	VLC_IMPORT(libvlc_media_player_pause, MediaPlayerPause);
	VLC_IMPORT(libvlc_media_player_play, MediaPlayerPlay);
	VLC_IMPORT(libvlc_media_player_set_pause, MediaPlayerSetPause);
	VLC_IMPORT(libvlc_media_player_stop, MediaPlayerStop);
	VLC_IMPORT(libvlc_media_player_will_play, MediaPlayerWillPlay);

	VLC_IMPORT(libvlc_audio_set_callbacks, AudioSetCallbacks);
	VLC_IMPORT(libvlc_audio_set_format, AudioSetFormat);
	VLC_IMPORT(libvlc_audio_set_format_callbacks, AudioSetFormatCallbacks);
	VLC_IMPORT(libvlc_audio_get_track, AudioGetTrack);
	VLC_IMPORT(libvlc_audio_set_track, AudioSetTrack);

	VLC_IMPORT(libvlc_video_get_height, VideoGetHeight);
	VLC_IMPORT(libvlc_video_get_width, VideoGetWidth);
	VLC_IMPORT(libvlc_video_set_callbacks, VideoSetCallbacks);
	VLC_IMPORT(libvlc_video_set_format, VideoSetFormat);
	VLC_IMPORT(libvlc_video_set_format_callbacks, VideoSetFormatCallbacks);
	VLC_IMPORT(libvlc_video_get_spu, VideoGetSpu);
	VLC_IMPORT(libvlc_video_set_spu, VideoSetSpu);
	VLC_IMPORT(libvlc_video_get_track, VideoGetTrack);
	VLC_IMPORT(libvlc_video_set_track, VideoSetTrack);

	VLC_IMPORT(libvlc_audio_get_track_description, AudioGetTrackDescription);
	VLC_IMPORT(libvlc_video_get_spu_description, VideoGetSpuDescription);
	VLC_IMPORT(libvlc_video_get_track_description, VideoGetTrackDescription);
	VLC_IMPORT(libvlc_track_description_release, TrackDescriptionListRelease);

	PluginDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(*LibDir, TEXT("plugins")));

	return true;
}


void FVlc::Shutdown()
{
	PluginDir.Empty();

	FreeDependency(LibHandle);
	FreeDependency(CoreHandle);

#if PLATFORM_WINDOWS
	FreeDependency(GccHandle);
#endif
}


/* FVlc static functions
 *****************************************************************************/

void FVlc::FreeDependency(void*& Handle)
{
	if (Handle != nullptr)
	{
		FPlatformProcess::FreeDllHandle(Handle);
		Handle = nullptr;
	}
}


bool FVlc::LoadDependency(const FString& Dir, const FString& Name, void*& Handle)
{
	FString Lib = Name + TEXT(".") + FPlatformProcess::GetModuleExtension();
	FString Path = Dir.IsEmpty() ? *Lib : FPaths::Combine(*Dir, *Lib);

	Handle = FPlatformProcess::GetDllHandle(*Path);

	if (Handle == nullptr)
	{
		GLog->Logf(ELogVerbosity::Warning, TEXT("VlcMedia: Failed to load required library %s. Plug-in will not be functional."), *Lib);
		return false;
	}

	return true;
}
