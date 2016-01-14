// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#pragma once

#include "ModuleManager.h"


/**
 * Implements a media player using the Video LAN Codec (VLC) framework.
 */
class FVlcMediaPlayer
	: public IMediaInfo
	, public IMediaPlayer
{
public:

	/**
	 * Create and initialize a new instance.
	 *
	 * @param InInstance The LibVLC instance to use. 
	 */
	FVlcMediaPlayer(FLibvlcInstance* InInstance);

	/** Destructor. */
	~FVlcMediaPlayer();

public:

	// IMediaInfo interface

	virtual FTimespan GetDuration() const override;
	virtual TRange<float> GetSupportedRates( EMediaPlaybackDirections Direction, bool Unthinned ) const override;
	virtual FString GetUrl() const override;
	virtual bool SupportsRate( float Rate, bool Unthinned ) const override;
	virtual bool SupportsScrubbing() const override;
	virtual bool SupportsSeeking() const override;

public:

	// IMediaPlayer interface

	virtual void Close() override;
	virtual const TArray<IMediaAudioTrackRef>& GetAudioTracks() const override;
	virtual const TArray<IMediaCaptionTrackRef>& GetCaptionTracks() const override;
	virtual const IMediaInfo& GetMediaInfo() const override;
	virtual float GetRate() const override;
	virtual FTimespan GetTime() const override;
	virtual const TArray<IMediaVideoTrackRef>& GetVideoTracks() const override;
	virtual bool IsLooping() const override;
	virtual bool IsPaused() const override;
	virtual bool IsPlaying() const override;
	virtual bool IsReady() const override;
	virtual bool Open(const FString& Url ) override;
	virtual bool Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl) override;
	virtual bool Seek(const FTimespan& Time) override;
	virtual bool SetLooping(bool Looping) override;
	virtual bool SetRate(float Rate) override;

	DECLARE_DERIVED_EVENT(FVlcMediaPlayer, IMediaPlayer::FOnMediaEvent, FOnMediaEvent);
	virtual FOnMediaEvent& OnMediaEvent() override
	{
		return MediaEvent;
	}

protected:

	/**
	 * Initialize the media player object.
	 *
	 * @param Media The media to play.
	 * @return true on success, false otherwise.
	 */
	bool InitializeMediaPlayer(FLibvlcMedia* Media);

	/** Initialize the media tracks. */
	void InitializeTracks();

private:

	/** Handles the ticker. */
	bool HandleTicker(float DeltaTime);

private:

	/** Handles event callbacks. */
	static void HandleEventCallback(FLibvlcEvent* Event, void* UserData);

	/** Handles open callbacks from VLC. */
	static int HandleMediaOpen(void* Opaque, void** OutData, uint64* OutSize);

	/** Handles read callbacks from VLC. */
	static SSIZE_T HandleMediaRead(void* Opaque, void* Buffer, SIZE_T Length);

	/** Handles seek callbacks from VLC. */
	static int HandleMediaSeek(void* Opaque, uint64 Offset);

	/** Handles close callbacks from VLC. */
	static void HandleMediaClose(void* Opaque);

private:

	/** The available audio tracks. */
	TArray<IMediaAudioTrackRef> AudioTracks;

	/** The available caption tracks. */
	TArray<IMediaCaptionTrackRef> CaptionTracks;

	/** Current playback time to work around VLC's broken time tracking. */
	FTimespan CurrentTime;

	/** The file or memory archive to stream from (for local media only). */
	TSharedPtr<FArchive, ESPMode::ThreadSafe> Data;

	/** The desired playback rate. */
	float DesiredRate;

	/** Collection of received player events. */
	TQueue<ELibvlcEventType, EQueueMode::Mpsc> Events;

	/** Platform time seconds at the last playback position change. */
	double LastPlatformSeconds;

	/** Holds an event delegate that is invoked when a media event occurred. */
	FOnMediaEvent MediaEvent;

	/** Currently opened media. */
	FString MediaUrl;

	/** The VLC media player object. */
	FLibvlcMediaPlayer* Player;

	/** Whether playback should be looping. */
	bool ShouldLoop;

	/** Handle to the registered ticker. */
	FDelegateHandle TickerHandle;

	/** Collection of all available tracks. */
	TArray<TSharedRef<FVlcMediaTrack, ESPMode::ThreadSafe>> Tracks;

	/** The available video tracks. */
	TArray<IMediaVideoTrackRef> VideoTracks;

	/** The LibVLC instance. */
	FLibvlcInstance* VlcInstance;
};
