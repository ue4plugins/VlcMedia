// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#pragma once

#include "IMediaInfo.h"
#include "IMediaPlayer.h"


struct FLibvlcMediaPlayer;
class FVlcMediaTrack;
class IMediaAudioTrack;
class IMediaCaptionTrack;
class IMediaVideoTrack;


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
	virtual TRange<float> GetSupportedRates(EMediaPlaybackDirections Direction, bool Unthinned) const override;
	virtual FString GetUrl() const override;
	virtual bool SupportsRate(float Rate, bool Unthinned) const override;
	virtual bool SupportsScrubbing() const override;
	virtual bool SupportsSeeking() const override;

public:

	// IMediaPlayer interface

	virtual void Close() override;
	virtual const TArray<TSharedRef<IMediaAudioTrack, ESPMode::ThreadSafe>>& GetAudioTracks() const override;
	virtual const TArray<TSharedRef<IMediaCaptionTrack, ESPMode::ThreadSafe>>& GetCaptionTracks() const override;
	virtual const IMediaInfo& GetMediaInfo() const override;
	virtual float GetRate() const override;
	virtual FTimespan GetTime() const override;
	virtual const TArray<TSharedRef<IMediaVideoTrack, ESPMode::ThreadSafe>>& GetVideoTracks() const override;
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
	 * @return true on success, false otherwise.
	 */
	bool InitializePlayer();

private:

	/** Handles the ticker. */
	bool HandleTicker(float DeltaTime);

private:

	/** Handles event callbacks. */
	static void HandleEventCallback(FLibvlcEvent* Event, void* UserData);

private:

	/** The audio callback handler. */
	FVlcMediaAudioHandler AudioHandler;

	/** The available caption tracks. */
	TArray<TSharedRef<IMediaCaptionTrack, ESPMode::ThreadSafe>> CaptionTracks;

	/** Current playback time to work around VLC's broken time tracking. */
	FTimespan CurrentTime;

	/** The desired playback rate. */
	float DesiredRate;

	/** Collection of received player events. */
	TQueue<ELibvlcEventType, EQueueMode::Mpsc> Events;

	/** Platform time seconds at the last playback position change. */
	double LastPlatformSeconds;

	/** Holds an event delegate that is invoked when a media event occurred. */
	FOnMediaEvent MediaEvent;

	/** The media source (from URL or archive). */
	FVlcMediaSource MediaSource;

	/** The VLC media player object. */
	FLibvlcMediaPlayer* Player;

	/** Whether playback should be looping. */
	bool ShouldLoop;

	/** Handle to the registered ticker. */
	FDelegateHandle TickerHandle;

	/** The video callback handler. */
	FVlcMediaVideoHandler VideoHandler;
};
