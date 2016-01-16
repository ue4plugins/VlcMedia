// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once


class FVlcMediaVideoTrack;


/**
 * Handles video related media player callbacks.
 */
class FVlcMediaVideoHandler
{
public:

	/** Default constructor. */
	FVlcMediaVideoHandler();

public:

	/**
	 * Get the available video tracks.
	 *
	 * @return Collection of tracks.
	 */
	const TArray<TSharedRef<IMediaVideoTrack, ESPMode::ThreadSafe>>& GetTracks() const;

	/**
	 * Initialize this handler for the specified media player.
	 *
	 * @param InPlayer The media player that owns this handler.
	 */
	void Initialize(FLibvlcMediaPlayer* InPlayer);

	/** Initialize the video tracks. */
	void InitializeTracks();

	/** Set the current playback time. */
	void SetTime(FTimespan Time)
	{
		CurrentTime = Time;
	}

	/** Shut down this handler. */
	void Shutdown();

private:

	/** Handles the display callback from VLC. */
	static void HandleVideoDisplay(void* Opaque, void* Picture);

	/** Handles the buffer lock callback from VLC. */
	static void* HandleVideoLock(void* Opaque, void** Planes);

	/** Handles the buffer unlock callback from VLC. */
	static void HandleVideoUnlock(void* Opaque, void* Picture, void* const* Planes);

private:

	/** Critical section for locking access to Tracks. */
	mutable FCriticalSection CriticalSection;

	/** The current playback time. */
	FTimespan CurrentTime;

	/** The video track that is currently enabled. */
	TWeakPtr<FVlcMediaVideoTrack, ESPMode::ThreadSafe> CurrentVideoTrack;

	/** The VLC media player object. */
	FLibvlcMediaPlayer* Player;

	/** The available video tracks. */
	TArray<TSharedRef<IMediaVideoTrack, ESPMode::ThreadSafe>> Tracks;

	/** Whether the tracks have been initialized yet. */
	bool TracksInitialized;
};
