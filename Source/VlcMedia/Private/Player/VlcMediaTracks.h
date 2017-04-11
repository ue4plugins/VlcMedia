// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IMediaTracks.h"


struct FLibvlcMediaPlayer;


/**
 * Implements the track collection for VLC based media players.
 */
class FVlcMediaTracks
	: public IMediaTracks
{
	struct FTrack
	{
		FText DisplayName;
		FString Name;
	}; 

public:

	/** Default constructor. */
	FVlcMediaTracks();

public:

	/**
	 * Initialize this object for the specified VLC media player.
	 *
	 * @param InPlayer The VLC media player.
	 * @param OutInfo Will contain information about the available media tracks.
	 */
	void Initialize(FLibvlcMediaPlayer& InPlayer, FString& OutInfo);

	/** Shut down this object. */
	void Shutdown();

public:

	//~ IMediaTracks interface

	virtual uint32 GetAudioTrackChannels(int32 TrackIndex) const override;
	virtual uint32 GetAudioTrackSampleRate(int32 TrackIndex) const override;
	virtual int32 GetNumTracks(EMediaTrackType TrackType) const override;
	virtual int32 GetSelectedTrack(EMediaTrackType TrackType) const override;
	virtual FText GetTrackDisplayName(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual FString GetTrackLanguage(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual FString GetTrackName(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual uint32 GetVideoTrackBitRate(int32 TrackIndex) const override;
	virtual FIntPoint GetVideoTrackDimensions(int32 TrackIndex) const override;
	virtual float GetVideoTrackFrameRate(int32 TrackIndex) const override;
	virtual bool SelectTrack(EMediaTrackType TrackType, int32 TrackIndex) override;

private:

	/** Audio track descriptors. */
	TArray<FTrack> AudioTracks;

	/** Caption track descriptors. */
	TArray<FTrack> CaptionTracks;

	/** The VLC media player object. */
	FLibvlcMediaPlayer* Player;

	/** Video track descriptors. */
	TArray<FTrack> VideoTracks;
};
