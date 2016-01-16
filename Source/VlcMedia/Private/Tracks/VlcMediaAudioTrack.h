// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IMediaAudioTrack.h"
#include "VlcMediaTrack.h"


struct FLibvlcMediaPlayer;
struct FLibvlcTrackDescription;
class IMediaStream;


class FVlcMediaAudioTrack
	: public FVlcMediaTrack
	, public IMediaAudioTrack
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InPlayer The VLC media player instance that owns this track.
	 * @param Descr The track description.
	 */
    FVlcMediaAudioTrack(FLibvlcMediaPlayer* InPlayer, FLibvlcTrackDescription* Descr);

	/** Virtual destructor. */
	virtual ~FVlcMediaAudioTrack();

public:

	void ProcessSamples(void* Samples, uint32 Count)
	{
		ProcessMediaSample(Samples, Count * NumChannels * sizeof(int16), 0.0f);
	}

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

	/** The number of channels. */
	uint32 NumChannels;

	/** The number of samples per second. */
	uint32 SamplesPerSecond;
};
