// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IMediaVideoTrack.h"
#include "VlcMediaTrack.h"


struct FLibvlcMediaPlayer;
struct FLibvlcTrackDescription;
class FRHITexture;
class IMediaStream;


class FVlcMediaVideoTrack
	: public FVlcMediaTrack
	, public IMediaVideoTrack
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

	// IMediaVideoTrack interface

	virtual uint32 GetBitRate() const override;
	virtual FIntPoint GetDimensions() const override;
	virtual float GetFrameRate() const override;
	virtual IMediaStream& GetStream() override;

#if WITH_ENGINE
	virtual void BindTexture(FRHITexture* Texture) override;
	virtual void UnbindTexture(FRHITexture* Texture) override;
#endif

public:

	// IMediaStream interface

    virtual bool Disable() override;
    virtual bool Enable() override;
    virtual bool IsEnabled() const override;

private:

	/** Handles the buffer lock callback from VLC. */
	static void* HandleVideoLock(void* Opaque, void** Planes);

	/** Handles the buffer unlock callback from VLC. */
	static void HandleVideoUnlock(void* Opaque, void* Picture, void* const* Planes);

	/** Handles the display callback from VLC. */
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
