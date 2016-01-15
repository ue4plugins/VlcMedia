// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IMediaCaptionTrack.h"
#include "VlcMediaTrack.h"


struct FLibvlcMediaPlayer;
struct FLibvlcTrackDescription;
class IMediaStream;


class FVlcMediaCaptionTrack
	: public FVlcMediaTrack
	, public IMediaCaptionTrack
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InPlayer The VLC media player instance that owns this track.
	 * @param Descr The track description.
	 */
	FVlcMediaCaptionTrack(FLibvlcMediaPlayer* InPlayer, FLibvlcTrackDescription* Descr);

	/** Virtual destructor. */
	virtual ~FVlcMediaCaptionTrack() { }

public:

	// IMediaCaptionTrack interface

	virtual IMediaStream& GetStream() override;

public:

	// IMediaStream interface

    virtual bool Disable() override;
    virtual bool Enable() override;
	virtual bool IsEnabled() const override;

private:

	/** The caption track's ID. */
	int32 SpuId;
};
