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

	/** Handles audio cleanup callbacks from VLC. */
	static void HandleAudioCleanup(void* Opaque);

	/** Handles audio drain callbacks from VLC. */
	static void HandleAudioDrain(void* Opaque);

	/** Handles audio flush callbacks from VLC. */
	static void HandleAudioFlush(void* Opaque, int64 Timestamp);

	/** Handles audio pause callbacks from VLC. */
	static void HandleAudioPause(void* Opaque, int64 Timestamp);

	/** Handles audio play callbacks from VLC. */
	static void HandleAudioPlay(void* Opaque, void* Samples, uint32 Count, int64 Timestamp);

	/** Handles audio resume callbacks from VLC. */
	static void HandleAudioResume(void* Opaque, int64 Timestamp);

	/** Handles audio setup callbacks from VLC. */
	static int HandleAudioSetup(void** Opaque, ANSICHAR* Format, uint32* Rate, uint32* Channels);

private:

	/** The audio track's ID. */
	int32 AudioTrackId;

	/** The number of channels. */
	uint32 NumChannels;

	/** The number of samples per second. */
	uint32 SamplesPerSecond;
};
