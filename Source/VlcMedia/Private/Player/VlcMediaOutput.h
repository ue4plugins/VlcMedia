// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IMediaOutput.h"


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
	 * Set the current playback time.
	 *
	 * @param Time The time to set.
	 */
	void SetTime(FTimespan Time)
	{
		CurrentTime = Time;
	}

	/** Shut down this handler. */
	void Shutdown();

public:

	//~ IMediaOutput interface

	virtual void SetAudioSink(IMediaAudioSink* Sink) override;
	virtual void SetCaptionSink(IMediaStringSink* Sink) override;
	virtual void SetImageSink(IMediaTextureSink* Sink) override;
	virtual void SetVideoSink(IMediaTextureSink* Sink) override;

protected:

	void SetupAudioOutput();
	void SetupCaptionOutput();
	void SetupVideoOutput();

private:

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

	/** Handles video cleanup callbacks from VLC. */
	static void StaticVideoCleanupCallback(void *Opaque);

	/** Handles display callbacks from VLC. */
	static void StaticVideoDisplayCallback(void* Opaque, void* Picture);

	/** Handles video format callbacks from VLC. */
	static unsigned StaticVideoFormatCallback(void** Opaque, char* Chroma, unsigned* Width, unsigned* Height, unsigned* Pitches, unsigned* Lines);

	/** Handles buffer lock callbacks from VLC. */
	static void* StaticVideoLockCallback(void* Opaque, void** Planes);

	/** Handles buffer unlock callbacks from VLC. */
	static void StaticVideoUnlockCallback(void* Opaque, void* Picture, void* const* Planes);

private:

	/** The audio sink. */
	IMediaAudioSink* AudioSink;

	/** The caption text sink. */
	IMediaStringSink* CaptionSink;

	/** Critical section for synchronizing access to sinks. */
	FCriticalSection CriticalSection;

	/** The current playback time. */
	FTimespan CurrentTime;

	/** The VLC media player object. */
	FLibvlcMediaPlayer* Player;

	/** Dimensions of the current video track. */
	FIntPoint VideoDimensions;

	/** The video sink. */
	IMediaTextureSink* VideoSink;
};