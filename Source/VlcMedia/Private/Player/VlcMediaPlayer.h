// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IMediaControls.h"
#include "IMediaPlayer.h"
#include "VlcMediaOutput.h"
#include "VlcMediaSource.h"
#include "VlcMediaTracks.h"
#include "Containers/Queue.h"


class IMediaOutput;

enum class ELibvlcEventType;

struct FLibvlcEvent;
struct FLibvlcEventManager;
struct FLibvlcInstance;
struct FLibvlcMediaPlayer;


/**
 * Implements a media player using the Video LAN Codec (VLC) framework.
 */
class FVlcMediaPlayer
	: public IMediaControls
	, public IMediaPlayer
{
public:

	/**
	 * Create and initialize a new instance.
	 *
	 * @param InInstance The LibVLC instance to use.
	 */
	FVlcMediaPlayer(FLibvlcInstance* InInstance);

	/** Virtual destructor. */
	virtual ~FVlcMediaPlayer();

public:

	//~ IMediaControls interface

	virtual FTimespan GetDuration() const override;
	virtual float GetRate() const override;
	virtual EMediaState GetState() const override;
	virtual TRange<float> GetSupportedRates(EMediaPlaybackDirections Direction, bool Unthinned) const override;
	virtual FTimespan GetTime() const override;
	virtual bool IsLooping() const override;
	virtual bool Seek(const FTimespan& Time) override;
	virtual bool SetLooping(bool Looping) override;
	virtual bool SetRate(float Rate) override;
	virtual bool SupportsRate(float Rate, bool Unthinned) const override;
	virtual bool SupportsScrubbing() const override;
	virtual bool SupportsSeeking() const override;

public:

	//~ IMediaPlayer interface

	virtual void Close() override;
	virtual IMediaControls& GetControls() override;
	virtual FString GetInfo() const override;
	virtual FName GetName() const override;
	virtual IMediaOutput& GetOutput() override;
	virtual FString GetStats() const override;
	virtual IMediaTracks& GetTracks() override;
	virtual FString GetUrl() const override;
	virtual bool Open(const FString& Url, const IMediaOptions& Options) override;
	virtual bool Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl, const IMediaOptions& Options) override;
	virtual void TickPlayer(float DeltaTime) override;
	virtual void TickVideo(float DeltaTime) override;

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

	/** Handles event callbacks. */
	static void StaticEventCallback(FLibvlcEvent* Event, void* UserData);

private:

	/** Current playback time (to work around VLC's broken time tracking). */
	FTimespan CurrentTime;

	/** Current difference between reported and interpolated time. */
	FTimespan CurrentTimeDrift;

	/** The desired playback rate. */
	float DesiredRate;

	/** Collection of received player events. */
	TQueue<ELibvlcEventType, EQueueMode::Mpsc> Events;

	/** Media information string. */
	FString Info;

	/** Platform time seconds at the last playback position change. */
	double LastPlatformSeconds;

	/** Holds an event delegate that is invoked when a media event occurred. */
	FOnMediaEvent MediaEvent;

	/** The media source (from URL or archive). */
	FVlcMediaSource MediaSource;

	/** Output manager. */
	FVlcMediaOutput Output;

	/** The VLC media player object. */
	FLibvlcMediaPlayer* Player;

	/** Whether playback should be looping. */
	bool ShouldLoop;

	/** Track collection. */
	FVlcMediaTracks Tracks;
};
