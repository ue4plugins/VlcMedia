// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IMediaOutput.h"


class IMediaOptions;
struct FLibvlcMediaPlayer;


/**
 * Handles media tracks and event callbacks.
 */
class FVlcMediaOutput
	: public IMediaOutput
{
public:

	/** Default constructor. */
	FVlcMediaOutput();

public:

	/**
	 * Initialize this handler for the specified media player.
	 *
	 * @param InPlayer The media player that owns this handler.
	 */
	void Initialize(FLibvlcMediaPlayer& InPlayer);

	/**
	 * Notify sinks that playback was resumed.
	 *
	 * LibVLC does not call StaticAudioResumeCallback when playback
	 * beings, so we're resuming the audio sink via this method.
	 *
	 * @param InResumeTime Time at which playback resumed.
	 */
	void Resume(FTimespan InResumeTime);

	/** Shut down this handler. */
	void Shutdown();

public:

	//~ IMediaOutput interface

	virtual void SetAudioSink(IMediaAudioSink* Sink) override;
	virtual void SetMetadataSink(IMediaBinarySink* Sink) override;
	virtual void SetOverlaySink(IMediaOverlaySink* Sink) override;
	virtual void SetVideoSink(IMediaTextureSink* Sink) override;

protected:

	/** Set up audio related callbacks. */
	void SetupAudioOutput();

	/** Set up caption related callbacks. */
	void SetupCaptionOutput();

	/** Set up subtitle related callbacks. */
	void SetupSubtitleOutput();

	/** Set up video related callbacks. */
	void SetupVideoOutput();

	/**
	 * Convert a VLC timestamp to a playback timespan.
	 *
	 * @param Timestamp The timestamp to convert.
	 * @return The corresponding timespan.
	 */
	FTimespan TimestampToTimespan(int64 Timestamp)
	{
		return ResumeTime + FTimespan::FromMicroseconds(Timestamp - ResumeOrigin);
	}

private:

	/** Handles audio cleanup callbacks from VLC.*/
	static void StaticAudioCleanupCallback(void* Opaque);

	/** Handles audio drain callbacks from VLC. */
	static void StaticAudioDrainCallback(void* Opaque);

	/** Handles audio flush callbacks from VLC. */
	static void StaticAudioFlushCallback(void* Opaque, int64 Timestamp);

	/** Handles audio pause callbacks from VLC. */
	static void StaticAudioPauseCallback(void* Opaque, int64 Timestamp);

	/** Handles audio play callbacks from VLC. */
	static void StaticAudioPlayCallback(void* Opaque, void* Samples, uint32 Count, int64 Timestamp);

	/** Handles audio resume callbacks from VLC. */
	static void StaticAudioResumeCallback(void* Opaque, int64 Timestamp);

	/** Handles audio setup callbacks from VLC. */
	static int StaticAudioSetupCallback(void** Opaque, ANSICHAR* Format, uint32* Rate, uint32* Channels);

	/** Handles video cleanup callbacks from VLC. */
	static void StaticVideoCleanupCallback(void *Opaque);

	/** Handles display callbacks from VLC. */
	static void StaticVideoDisplayCallback(void* Opaque, void* Picture);

	/** Handles buffer lock callbacks from VLC. */
	static void* StaticVideoLockCallback(void* Opaque, void** Planes);

	/** Handles video setup callbacks from VLC. */
	static unsigned StaticVideoSetupCallback(void** Opaque, char* Chroma, unsigned* Width, unsigned* Height, unsigned* Pitches, unsigned* Lines);

	/** Handles buffer unlock callbacks from VLC. */
	static void StaticVideoUnlockCallback(void* Opaque, void* Picture, void* const* Planes);

private:

	/** The audio sink. */
	IMediaAudioSink* AudioSink;
	
	/** Critical section for synchronizing access to sinks. */
	FCriticalSection CriticalSection;

	/** The VLC media player object. */
	FLibvlcMediaPlayer* Player;

	/** Origin of timestamps. */
	int64 ResumeOrigin;

	/** The time at which playback resumed. */
	FTimespan ResumeTime;

	/** The text overlay sink. */
	IMediaOverlaySink* OverlaySink;

	/** Dimensions of the current video track. */
	FIntPoint VideoDimensions;

	/** The video sink. */
	IMediaTextureSink* VideoSink;
};