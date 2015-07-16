// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once


class FVlcMediaAudioTrack
	: public FVlcMediaTrack
	, public IMediaAudioTrack
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InPlayer The VLC media player instance that owns this track.
	 * @param InTrackIndex The index number of this track.
	 * @param Descr The track description.
	 */
    FVlcMediaAudioTrack(FLibvlcMediaPlayer* InPlayer, uint32 InTrackIndex, FLibvlcTrackDescription* Descr);

	/** Virtual destructor. */
	virtual ~FVlcMediaAudioTrack() { }


public:

	// IMediaAudioTrack interface

	virtual uint32 GetNumChannels() const override;
	virtual uint32 GetSamplesPerSecond() const override;
	virtual IMediaStream& GetStream() override;

public:

	// IMediaStream interface

    virtual bool Disable() override;
    virtual bool Enable() override;
	virtual bool IsEnabled() const override;

private:

	/** The audio track's ID. */
	int32 AudioTrackId;
};
