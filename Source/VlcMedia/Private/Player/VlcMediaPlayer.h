// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "IMediaCache.h"
#include "IMediaControls.h"
#include "IMediaPlayer.h"
#include "IMediaSamples.h"

#include "VlcMediaCallbacks.h"
#include "VlcMediaSource.h"
#include "VlcMediaTracks.h"
#include "VlcMediaView.h"

class IMediaEventSink;
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
	: public IMediaPlayer
	, protected IMediaCache
	, protected IMediaControls
{
public:

	/**
	 * Create and initialize a new instance.
	 *
	 * @param InEventSink The object that receives media events from this player.
	 * @param InInstance The LibVLC instance to use.
	 */
	FVlcMediaPlayer(IMediaEventSink& InEventSink, FLibvlcInstance* InInstance);

	/** Virtual destructor. */
	virtual ~FVlcMediaPlayer();

public:

	//~ IMediaPlayer interface

	virtual void Close() override;
	virtual IMediaCache& GetCache() override;
	virtual IMediaControls& GetControls() override;
	virtual FString GetInfo() const override;
	virtual FName GetPlayerName() const override;
	virtual IMediaSamples& GetSamples() override;
	virtual FString GetStats() const override;
	virtual IMediaTracks& GetTracks() override;
	virtual FString GetUrl() const override;
	virtual IMediaView& GetView() override;
	virtual bool Open(const FString& Url, const IMediaOptions* Options) override;
	virtual bool Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl, const IMediaOptions* Options) override;
	virtual void TickInput(FTimespan DeltaTime, FTimespan Timecode) override;

protected:

	/**
	 * Initialize the media player.
	 *
	 * @return true on success, false otherwise.
	 */
	bool InitializePlayer();

protected:

	//~ IMediaControls interface

	virtual bool CanControl(EMediaControl Control) const override;
	virtual FTimespan GetDuration() const override;
	virtual float GetRate() const override;
	virtual EMediaState GetState() const override;
	virtual EMediaStatus GetStatus() const override;
	virtual TRangeSet<float> GetSupportedRates(EMediaRateThinning Thinning) const override;
	virtual FTimespan GetTime() const override;
	virtual bool IsLooping() const override;
	virtual bool Seek(const FTimespan& Time) override;
	virtual bool SetLooping(bool Looping) override;
	virtual bool SetRate(float Rate) override;

private:

	/** Handles event callbacks. */
	static void StaticEventCallback(FLibvlcEvent* Event, void* UserData);

private:

	/** VLC callback manager. */
	FVlcMediaCallbacks Callbacks;

	/** Current playback rate. */
	float CurrentRate;

	/** Current playback time (to work around VLC's broken time tracking). */
	FTimespan CurrentTime;

	/** The media event handler. */
	IMediaEventSink& EventSink;

	/** Collection of received player events. */
	TQueue<ELibvlcEventType, EQueueMode::Mpsc> Events;

	/** Media information string. */
	FString Info;

	/** The media source (from URL or archive). */
	FVlcMediaSource MediaSource;

	/** The VLC media player object. */
	FLibvlcMediaPlayer* Player;

	/** Whether playback should be looping. */
	bool ShouldLoop;

	/** Track collection. */
	FVlcMediaTracks Tracks;

	/** View settings. */
	FVlcMediaView View;
};
