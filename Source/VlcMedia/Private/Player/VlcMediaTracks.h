// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IMediaTracks.h"


struct FLibvlcMediaPlayer;


/**
 * Track descriptor.
 */
struct FVlcMediaTrack
{
	/** The track's human readable name. */
	FText DisplayName;

	/** The track's name. */
	FString Name;
};


/**
 * Implements the track collection for VLC based media players.
 */
class FVlcMediaTracks
	: public IMediaTracks
{
public:

	/** Default constructor. */
	FVlcMediaTracks();

public:

	/**
	 * Initialize this object for the specified VLC media player.
	 *
	 * @param InPlayer The VLC media player.
	 */
	void Initialize(FLibvlcMediaPlayer& InPlayer);

	/** Shut down this object. */
	void Shutdown();

public:

	//~ IMediaTracks interface

	virtual uint32 GetAudioTrackChannels(int32 TrackIndex) const override;
	virtual uint32 GetAudioTrackSamplesPerSeconds(int32 TrackIndex) const override;
	virtual int32 GetNumTracks(EMediaTrackType TrackType) const override;
	virtual int32 GetSelectedTrack(EMediaTrackType TrackType) const override;
	virtual FText GetTrackDisplayName(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual FString GetTrackLanguage(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual FString GetTrackName(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual uint32 GetVideoTrackBitRate(int32 VideoTrackIndex) const override;
	virtual FIntPoint GetVideoTrackDimensions(int32 VideoTrackIndex) const override;
	virtual float GetVideoTrackFrameRate(int32 TrackIndex) const override;
	virtual bool SelectTrack(EMediaTrackType TrackType, int32 TrackIndex) override;

private:

	/** Audio track descriptors. */
	TArray<FVlcMediaTrack> AudioTracks;

	/** The VLC media player object. */
	FLibvlcMediaPlayer* Player;

	/** Video track descriptors. */
	TArray<FVlcMediaTrack> VideoTracks;
};
