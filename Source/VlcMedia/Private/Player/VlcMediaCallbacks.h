// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IMediaAudioSample.h"
#include "IMediaTextureSample.h"

class FMediaSamples;
class FVlcMediaAudioSamplePool;
class FVlcMediaTextureSamplePool;
class IMediaOptions;
class IMediaAudioSink;
class IMediaOverlaySink;
class IMediaSamples;
class IMediaTextureSink;

struct FLibvlcMediaPlayer;


/**
 * Handles VLC callbacks.
 */
class FVlcMediaCallbacks
{
public:

	/** Default constructor. */
	FVlcMediaCallbacks();

	/** Virtual destructor. */
	~FVlcMediaCallbacks();

public:

	/**
	 * Get the output media samples.
	 *
	 * @return Media samples interface.
	 */
	IMediaSamples& GetSamples();

	/**
	 * Initialize the handler for the specified media player.
	 *
	 * @param InPlayer The media player that owns this handler.
	 */
	void Initialize(FLibvlcMediaPlayer& InPlayer);

	/**
	 * Set the player's current time.
	 *
	 * @param Time The player's play time.
	 */
	void SetCurrentTime(FTimespan Time)
	{
		CurrentTime = Time;
	}

	/** Shut down the callback handler. */
	void Shutdown();

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

	/** Current number of channels in audio samples( accessed by VLC thread only). */
	uint32 AudioChannels;

	/** Current audio sample format (accessed by VLC thread only). */
	EMediaAudioSampleFormat AudioSampleFormat;

	/** Audio sample object pool. */
	FVlcMediaAudioSamplePool* AudioSamplePool;

	/** Current audio sample rate (accessed by VLC thread only). */
	uint32 AudioSampleRate;

	/** Size of a single audio sample (in bytes). */
	SIZE_T AudioSampleSize;

	/** The player's current time. */
	FTimespan CurrentTime;

	/** The VLC media player object. */
	FLibvlcMediaPlayer* Player;

	/** The output media samples. */
	FMediaSamples* Samples;

	/** Current video buffer dimensions (accessed by VLC thread only; may be larger than VideoOutputDim). */
	FIntPoint VideoBufferDim;

	/** Number of bytes per row of video pixels. */
	uint32 VideoBufferStride;

	/** Current duration of video frames. */
	FTimespan VideoFrameDuration;

	/** Current video output dimensions (accessed by VLC thread only). */
	FIntPoint VideoOutputDim;

	/** Play time of the previous frame. */
	FTimespan VideoPreviousTime;

	/** Current video sample format (accessed by VLC thread only). */
	EMediaTextureSampleFormat VideoSampleFormat;

	/** Video sample object pool. */
	FVlcMediaTextureSamplePool* VideoSamplePool;
};