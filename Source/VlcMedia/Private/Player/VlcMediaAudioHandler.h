// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once


class FVlcMediaAudioTrack;


/**
 * Handles audio related media player callbacks.
 */
class FVlcMediaAudioHandler
{
public:

	/** Default constructor. */
	FVlcMediaAudioHandler();

public:

	/**
	 * Get the available audio tracks.
	 *
	 * @return Collection of tracks.
	 */
	const TArray<TSharedRef<IMediaAudioTrack, ESPMode::ThreadSafe>>& GetTracks() const;

	/**
	 * Initialize this handler for the specified media player.
	 *
	 * @param InPlayer The media player that owns this handler.
	 */
	void Initialize(FLibvlcMediaPlayer* InPlayer);

	/** Initialize the audio tracks. */
	void InitializeTracks();

	/** Set the current playback time. */
	void SetTime(FTimespan Time)
	{
		CurrentTime = Time;
	}

	/** Shut down this handler. */
	void Shutdown();

private:

	/** Handles audio drain callbacks from VLC. */
	static void HandleAudioDrain(void* Opaque);

	/** Handles audio flush callbacks from VLC. */
	static void HandleAudioFlush(void* Opaque, int64 Timestamp);

	/** Handles audio pause callbacks from VLC. */
	static void HandleAudioPause(void* Opaque, int64 Timestamp);

	/** Handles audio play callbacks from VLC. */
	static void HandleAudioPlay(void* Opaque, void* Samples, uint32 Count, int64 Timestamp);

	/** Handles audio resume callbacks from VLC. */
	static void HandleAudioResume(void* Opaque, int64 Timestamp);

private:

	/** Critical section for locking access to Tracks. */
	mutable FCriticalSection CriticalSection;

	/** The current playback time. */
	FTimespan CurrentTime;

	/** The audio track that is currently enabled. */
	TWeakPtr<FVlcMediaAudioTrack, ESPMode::ThreadSafe> CurrentAudioTrack;

	/** The VLC media player object. */
	FLibvlcMediaPlayer* Player;

	/** The available audio tracks. */
	TArray<TSharedRef<IMediaAudioTrack, ESPMode::ThreadSafe>> Tracks;

	/** Whether the tracks have been initialized yet. */
	bool TracksInitialized;
};
