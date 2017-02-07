// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IMediaAudioSample.h"
#include "IMediaOutput.h"
#include "IMediaTextureSample.h"

class IMediaOptions;
class IMediaAudioSink;
class IMediaOverlaySink;
class IMediaTextureSink;

struct FLibvlcMediaPlayer;


/**
 * Handles media tracks and event callbacks.
 */
class FVlcMediaOutput
	: public IMediaOutput
{
	/** */
	struct FVlcMediaTimeInfo
	{
		/** The player's current play rate. */
		float Rate;

		/** The player's current time when StartTimecode was set. */
		FTimespan StartOffset;

		/** Time code at which playback was started with the current play rate. */
		FTimespan StartTimecode;

		/** The player's current play time. */
		FTimespan Time;

		/** The current time code. */
		FTimespan Timecode;


		/** Default constructor. */
		FVlcMediaTimeInfo()
			: Rate(0.0f)
			, StartOffset(FTimespan::Zero())
			, StartTimecode(FTimespan::Zero())
			, Time(FTimespan::Zero())
			, Timecode(FTimespan::Zero())
		{ }
	};

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

	/** Shut down this handler. */
	void Shutdown();

	/**
	 * Update this output.
	 *
	 * @param Timecode The current time code.
	 * @param Time The player's play time.
	 * @param Rate The player's play rate.
	 */
	void Update(FTimespan Timecode, FTimespan Time, float Rate);

public:

	//~ IMediaOutput interface

	virtual bool SetAudioNative(bool Enabled) override;
	virtual void SetAudioNativeVolume(float Volume) override;
	virtual void SetAudioSink(TSharedPtr<IMediaAudioSink, ESPMode::ThreadSafe> Sink) override;
	virtual void SetMetadataSink(TSharedPtr<IMediaBinarySink, ESPMode::ThreadSafe> Sink) override;
	virtual void SetOverlaySink(TSharedPtr<IMediaOverlaySink, ESPMode::ThreadSafe> Sink) override;
	virtual void SetVideoSink(TSharedPtr<IMediaTextureSink, ESPMode::ThreadSafe> Sink) override;

protected:

	/**
	 * Flush the output sinks.
	 *
	 * @param Shutdown Whether the sinks should be shut down.
	 */
	void FlushSinks(bool Shutdown);

	/** Set up audio related callbacks. */
	void SetupAudioOutput();

	/** Set up overlay text related callbacks. */
	void SetupOverlayOutput();

	/** Set up video related callbacks. */
	void SetupVideoOutput();

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
	TWeakPtr<IMediaAudioSink, ESPMode::ThreadSafe> AudioSinkPtr;

	/** The text overlay sink. */
	TWeakPtr<IMediaOverlaySink, ESPMode::ThreadSafe> OverlaySinkPtr;

	/** The video sink. */
	TWeakPtr<IMediaTextureSink, ESPMode::ThreadSafe> VideoSinkPtr;

private:

	/** Current number of channels in audio samples( accessed by VLC thread only). */
	uint32 AudioChannels;

	/** Current audio sample format (accessed by VLC thread only). */
	EMediaAudioSampleFormat AudioSampleFormat;

	/** Current audio sample rate (accessed by VLC thread only). */
	uint32 AudioSampleRate;

	/** Size of a single audio sample (in bytes). */
	SIZE_T AudioSampleSize;

	/** The VLC media player object. */
	FLibvlcMediaPlayer* Player;

	/** Current time information (updated via Update). */
	TSharedPtr<FVlcMediaTimeInfo, ESPMode::ThreadSafe> TimeInfo;

	/** Current video buffer dimensions (accessed by VLC thread only; may be larger than VideoOutputDim). */
	FIntPoint VideoBufferDim;

	/** Number of bytes per row of video pixels. */
	uint32 VideoBufferStride;

	/** Current video output dimensions (accessed by VLC thread only). */
	FIntPoint VideoOutputDim;

	/** Play time of the previous frame. */
	FTimespan VideoPreviousTime;

	/** Current video sample format (accessed by VLC thread only). */
	EMediaTextureSampleFormat VideoSampleFormat;
};