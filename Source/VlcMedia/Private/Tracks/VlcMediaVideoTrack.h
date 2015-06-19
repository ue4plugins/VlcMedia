// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once


class FVlcMediaVideoTrack
	: public FVlcMediaTrack
	, public IMediaTrackVideoDetails
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InPlayer The VLC media player instance that owns this track.
	 * @param InTrackIndex The index number of this track.
	 * @param Descr The track description.
	 */
	FVlcMediaVideoTrack(FLibvlcMediaPlayer* InPlayer, uint32 InTrackIndex, FLibvlcTrackDescription* Descr);

	/** Virtual destructor. */
	virtual ~FVlcMediaVideoTrack();

public:

	// IMediaTrackVideoDetails interface

	virtual uint32 GetBitRate() const override;
	virtual FIntPoint GetDimensions() const override;
	virtual float GetFrameRate() const override;

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

	static void* HandleVideoLock(void* Opaque, void** Planes);
	static void HandleVideoUnlock(void* Opaque, void* Picture, void* const* Planes);
	static void HandleVideoDisplay(void* Opaque, void* Picture);

private:

	/** The video's dimensions. */
	FIntPoint Dimensions;

	/** Buffer to write frame data to. */
	TArray<uint8> FrameBuffer;

	/** Last delta time. */
	FTimespan LastDelta;

	/** The track's cached name. */
	FString Name;

	/** The video track's ID. */
	int32 VideoTrackId;
};
