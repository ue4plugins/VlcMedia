// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VlcImports.h"
#include "VlcTypes.h"


class FVlc
{
public:

	static const uint32 MaxPlanes = 5;

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

	static FLibvlcLogGetContextProc LogGetContext;
	static FLibvlcLogSetProc LogSet;
	static FLibvlcLogUnsetProc LogUnset;

	static FLibvlcFreeProc Free;
	static FLibvlcGetChangesetProc GetChangeset;
	static FLibvlcGetCompilerProc GetCompiler;
	static FLibvlcGetVersionProc GetVersion;

	static FLibvlcClockProc Clock;

	static FLibvlcMediaEventManagerProc MediaEventManager;
	static FLibvlcMediaGetDurationProc MediaGetDuration;
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

	static FLibvlcAudioGetTrackProc AudioGetTrack;
	static FLibvlcAudioSetCallbacksProc AudioSetCallbacks;
	static FLibvlcAudioSetFormatProc AudioSetFormat;
	static FLibvlcAudioSetFormatCallbacksProc AudioSetFormatCallbacks;
	static FLibvlcAudioSetTrackProc AudioSetTrack;

	static FLibvlcVideoGetHeightProc VideoGetHeight;
	static FLibvlcVideoGetSizeProc VideoGetSize;
	static FLibvlcVideoGetSpuProc VideoGetSpu;
	static FLibvlcVideoGetSpuCountProc VideoGetSpuCount;
	static FLibvlcVideoGetTrackProc VideoGetTrack;
	static FLibvlcVideoGetWidthProc VideoGetWidth;
	static FLibvlcVideoNewViewpointProc VideoNewViewpoint;
	static FLibvlcVideoSetCallbacksProc VideoSetCallbacks;
	static FLibvlcVideoSetFormatProc VideoSetFormat;
	static FLibvlcVideoSetFormatCallbacksProc VideoSetFormatCallbacks;
	static FLibvlcVideoSetSpuProc VideoSetSpu;
	static FLibvlcVideoSetTrackProc VideoSetTrack;
	static FLibvlcVideoUpdateViewpointProc VideoUpdateViewpoint;

	static FLibvlcAudioGetTrackDescriptionProc AudioGetTrackDescription;
	static FLibvlcVideoGetSpuDescriptionProc VideoGetSpuDescription;
	static FLibvlcVideoGetTrackDescriptionProc VideoGetTrackDescription;
	static FLibvlcTrackDescriptionListReleaseProc TrackDescriptionListRelease;

	static FLibvlcFourccGetChromaDescriptionProc FourccGetChromaDescription;

public:

	static int64 Delay(int64 Timestamp)
	{
		return Timestamp - Clock();
	}

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