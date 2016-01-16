// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"


/* FVlcMediaAudioHandler structors
*****************************************************************************/

FVlcMediaAudioHandler::FVlcMediaAudioHandler()
	: CurrentTime(FTimespan::Zero())
	, Player(nullptr)
	, TracksInitialized(false)
{ }


/* FVlcMediaAudioHandler interface
*****************************************************************************/

const TArray<TSharedRef<IMediaAudioTrack, ESPMode::ThreadSafe>>& FVlcMediaAudioHandler::GetTracks() const
{
	FScopeLock Lock(&CriticalSection);

	return Tracks;
}


void FVlcMediaAudioHandler::Initialize(FLibvlcMediaPlayer* InPlayer)
{
	Shutdown();

	Player = InPlayer;
	TracksInitialized = false;

	FVlc::AudioSetCallbacks(
		Player,
		&FVlcMediaAudioHandler::HandleAudioPlay,
		&FVlcMediaAudioHandler::HandleAudioPause,
		&FVlcMediaAudioHandler::HandleAudioResume,
		&FVlcMediaAudioHandler::HandleAudioFlush,
		&FVlcMediaAudioHandler::HandleAudioDrain,
		this
	);
}


void FVlcMediaAudioHandler::InitializeTracks()
{
	if (TracksInitialized)
	{
		return;
	}

	// @todo gmp: make audio specs configurable?
	FVlc::AudioSetFormat(Player, "S16N", 44100, 2);

	FScopeLock Lock(&CriticalSection);
	Tracks.Reset();

	FLibvlcTrackDescription* AudioTrackDescr = FVlc::AudioGetTrackDescription(Player);
	{
		while (AudioTrackDescr != nullptr)
		{
			if (AudioTrackDescr->Id != -1)
			{
				TSharedRef<FVlcMediaAudioTrack, ESPMode::ThreadSafe> NewTrack = MakeShareable(
					new FVlcMediaAudioTrack(Player, AudioTrackDescr)
				);

				Tracks.Add(NewTrack);

				// hack
				CurrentAudioTrack = NewTrack;
			}

			AudioTrackDescr = AudioTrackDescr->Next;
		}
	}
	FVlc::TrackDescriptionListRelease(AudioTrackDescr);

	TracksInitialized = true;
}


void FVlcMediaAudioHandler::Shutdown()
{
	if (Player != nullptr)
	{
		FVlc::AudioSetCallbacks(Player, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	}

	CurrentAudioTrack.Reset();
	Tracks.Reset();
}


/* FVlcMediaAudioTrack static functions
*****************************************************************************/

void FVlcMediaAudioHandler::HandleAudioDrain(void* Opaque)
{
}


void FVlcMediaAudioHandler::HandleAudioFlush(void* Opaque, int64 Timestamp)
{
}


void FVlcMediaAudioHandler::HandleAudioPause(void* Opaque, int64 Timestamp)
{
}


void FVlcMediaAudioHandler::HandleAudioPlay(void* Opaque, void* Samples, uint32 Count, int64 Timestamp)
{
	auto Handler = (FVlcMediaAudioHandler*)Opaque;

	if (Handler != nullptr)
	{
		auto CurrentAudioTrack = Handler->CurrentAudioTrack.Pin();

		if (CurrentAudioTrack.IsValid())
		{
			CurrentAudioTrack->ProcessSamples(Samples, Count);
		}
	}
}


void FVlcMediaAudioHandler::HandleAudioResume(void* Opaque, int64 Timestamp)
{
}
