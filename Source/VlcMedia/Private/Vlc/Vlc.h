// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#pragma once

#include "VlcImports.h"
#include "VlcTypes.h"


class FVlc
{
public:

	static FString GetPluginDir();
	static bool Initialize();
	static void Shutdown();

public:

	static FLibvlcNewProc New;
	static FLibvlcReleaseProc Release;
	static FLibvlcRetainProc Retain;

	static FLibvlcErrmsgProc Errmsg;
	static FLibvlcClearerrProc Clearerr;

	static FLibvlcEventAttachProc EventAttach;
	static FLibvlcEventAttachProc EventDetach;
	static FLibvlcEventTypeNameProc EventTypeName;

	static FLibvlcLogSetProc LogSet;
	static FLibvlcLogUnsetProc LogUnset;

	static FLibvlcMediaEventManagerProc MediaEventManager;
	static FLibvlcMediaGetStatsProc MediaGetStats;
	static FLibvlcMediaNewCallbacksProc MediaNewCallbacks;
	static FLibvlcMediaNewLocationProc MediaNewLocation;
	static FLibvlcMediaNewPathProc MediaNewPath;
	static FLibvlcMediaParseAsyncProc MediaParseAsync;
	static FLibvlcMediaReleaseProc MediaRelease;
	static FLibvlcMediaRetainProc MediaRetain;
	static FLibvlcMediaTracksGetProc MediaTracksGet;
	static FLibvlcMediaTracksReleaseProc MediaTracksRelease;

	static FLibvlcMediaPlayerEventManagerProc MediaPlayerEventManager;
	static FLibvlcMediaPlayerGetMediaProc MediaPlayerGetMedia;
	static FLibvlcMediaPlayerNewProc MediaPlayerNew;
	static FLibvlcMediaPlayerNewFromMediaProc MediaPlayerNewFromMedia;
	static FLibvlcMediaPlayerReleaseProc MediaPlayerRelease;
	static FLibvlcMediaPlayerRetainProc MediaPlayerRetain;
	static FLibvlcMediaPlayerSetMediaProc MediaPlayerSetMedia;

	static FLibvlcMediaPlayerCanPauseProc MediaPlayerCanPause;
	static FLibvlcMediaPlayerGetFpsProc MediaPlayerGetFps;
	static FLibvlcMediaPlayerGetLengthProc MediaPlayerGetLength;
	static FLibvlcMediaPlayerGetPositionProc MediaPlayerGetPosition;
	static FLibvlcMediaPlayerGetRateProc MediaPlayerGetRate;
	static FLibvlcMediaPlayerGetStateProc MediaPlayerGetState;
	static FLibvlcMediaPlayerGetTimeProc MediaPlayerGetTime;
	static FLibvlcMediaPlayerIsSeekableProc MediaPlayerIsSeekable;
	static FLibvlcMediaPlayerSetPositionProc MediaPlayerSetPosition;
	static FLibvlcMediaPlayerSetRateProc MediaPlayerSetRate;
	static FLibvlcMediaPlayerSetTimeProc MediaPlayerSetTime;

	static FLibvlcMediaPlayerIsPlayingProc MediaPlayerIsPlaying;
	static FLibvlcMediaPlayerPauseProc MediaPlayerPause;
	static FLibvlcMediaPlayerPlayProc MediaPlayerPlay;
	static FLibvlcMediaPlayerSetPauseProc MediaPlayerSetPause;
	static FLibvlcMediaPlayerStopProc MediaPlayerStop;
	static FLibvlcMediaPlayerWillPlayProc MediaPlayerWillPlay;

	static FLibvlcAudioSetCallbacksProc AudioSetCallbacks;
	static FLibvlcAudioSetFormatProc AudioSetFormat;
	static FLibvlcAudioSetFormatCallbacksProc AudioSetFormatCallbacks;
	static FLibvlcAudioGetTrackProc AudioGetTrack;
	static FLibvlcAudioSetTrackProc AudioSetTrack;

	static FLibvlcVideoGetHeightProc VideoGetHeight;
	static FLibvlcVideoGetWidthProc VideoGetWidth;
	static FLibvlcVideoSetCallbacksProc VideoSetCallbacks;
	static FLibvlcVideoSetFormatProc VideoSetFormat;
	static FLibvlcVideoSetFormatCallbacksProc VideoSetFormatCallbacks;
	static FLibvlcVideoGetSpuProc VideoGetSpu;
	static FLibvlcVideoSetSpuProc VideoSetSpu;
	static FLibvlcVideoGetTrackProc VideoGetTrack;
	static FLibvlcVideoSetTrackProc VideoSetTrack;

	static FLibvlcAudioGetTrackDescriptionProc AudioGetTrackDescription;
	static FLibvlcVideoGetSpuDescriptionProc VideoGetSpuDescription;
	static FLibvlcVideoGetTrackDescriptionProc VideoGetTrackDescription;
	static FLibvlcTrackDescriptionListReleaseProc TrackDescriptionListRelease;

protected:

	static void FreeDependency(void*& Handle);
	static bool LoadDependency(const FString& Dir, const FString& Name, void*& Handle);

private:

	static void* CoreHandle;
	static void* LibHandle;
	static FString PluginDir;

#if PLATFORM_WINDOWS
	static void* GccHandle;
#endif
};