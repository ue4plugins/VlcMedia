// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once


class FVlcMediaCaptionTrack
	: public FVlcMediaTrack
	, public IMediaTrackCaptionDetails
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InPlayer The VLC media player instance that owns this track.
	 * @param InTrackIndex The index number of this track.
	 * @param Descr The track description.
	 */
	FVlcMediaCaptionTrack(FLibvlcMediaPlayer* InPlayer, uint32 InTrackIndex, FLibvlcTrackDescription* Descr);

	/** Virtual destructor. */
	virtual ~FVlcMediaCaptionTrack() { }

public:

	// IMediaTrack interface

    virtual bool Disable() override;
    virtual bool Enable() override;
	virtual const IMediaTrackAudioDetails& GetAudioDetails() const override;
	virtual const IMediaTrackCaptionDetails& GetCaptionDetails() const override;
	virtual EMediaTrackTypes GetType() const override;
	virtual const IMediaTrackVideoDetails& GetVideoDetails() const override;
	virtual bool IsEnabled() const override;

private:

	/** The caption track's ID. */
	int32 SpuId;
};
