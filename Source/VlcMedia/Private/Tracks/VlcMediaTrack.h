// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once


class FVlcMediaPlayer;


/**
 * Abstract base class for media tracks using libVLC.
 */
class FVlcMediaTrack
	: public IMediaStream
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InPlayer The media player that owns this track.
	 * @param InTrackIndex The index number of this track.
	 */
    FVlcMediaTrack(FLibvlcMediaPlayer* InPlayer, uint32 InTrackIndex, FLibvlcTrackDescription* Descr);

public:

	void SetTime(float Time)
	{
		LastTime = Time;
	}

public:

	// IMediaStream interface

    virtual void AddSink(const IMediaSinkRef& Sink) override;
    virtual FText GetDisplayName() const override;
    virtual FString GetLanguage() const override;
    virtual FString GetName() const override;
    virtual bool IsMutuallyExclusive(const IMediaStreamRef& Other) const override;
    virtual bool IsProtected() const override;
    virtual void RemoveSink(const IMediaSinkRef& Sink) override;

protected:
	
	/**
	 * Get the VLC media player that owns this track.
	 *
	 * @return The media player.
	 */
	FLibvlcMediaPlayer* GetPlayer() const
	{
		return Player;
	}

	/**
	 * Process a media data sample.
	 *
	 * @param SampleBuffer The buffer containing the sample data.
	 * @param SampleSize The number of bytes in the buffer.
	 * @param SampleDuration The sample's duration.
	 */
    void ProcessMediaSample(const void* SampleBuffer, uint32 SampleSize, FTimespan SampleDuration);

private:

	/** The track's human readable name. */
	FText DisplayName;

	/** Last known playback time as a workaround for VLC's broken time management. */
	float LastTime;

	/** The track's name. */
	FString Name;

	/** The VLC media player that owns this track. */
	FLibvlcMediaPlayer* Player;

	/** The collection of registered media sinks. */
    TArray<IMediaSinkWeakPtr> Sinks;

	/** The track's index number. */
    uint32 TrackIndex;
};
