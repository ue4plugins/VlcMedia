// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"


/* FVlcMediaVideoHandler structors
*****************************************************************************/

FVlcMediaVideoHandler::FVlcMediaVideoHandler()
	: CurrentTime(FTimespan::Zero())
	, Player(nullptr)
	, TracksInitialized(false)
{ }


/* FVlcMediaVideoHandler interface
*****************************************************************************/

const TArray<TSharedRef<IMediaVideoTrack, ESPMode::ThreadSafe>>& FVlcMediaVideoHandler::GetTracks() const
{
	FScopeLock Lock(&CriticalSection);

	return Tracks;
}


void FVlcMediaVideoHandler::Initialize(FLibvlcMediaPlayer* InPlayer)
{
	Shutdown();

	Player = InPlayer;
	TracksInitialized = false;

	FVlc::VideoSetCallbacks(
		Player,
		&FVlcMediaVideoHandler::HandleVideoLock,
		&FVlcMediaVideoHandler::HandleVideoUnlock,
		&FVlcMediaVideoHandler::HandleVideoDisplay,
		this
	);
}


void FVlcMediaVideoHandler::InitializeTracks()
{
	if (TracksInitialized)
	{
		return;
	}

	int32 Width = FVlc::VideoGetWidth(Player);
	int32 Height = FVlc::VideoGetHeight(Player);

	// @todo gmp: allow other pixel formats
	FVlc::VideoSetFormat(Player, "RV32", Width, Height, Width * 4);

	FScopeLock Lock(&CriticalSection);
	Tracks.Reset();

	FLibvlcTrackDescription* VideoTrackDescr = FVlc::VideoGetTrackDescription(Player);
	{
		while (VideoTrackDescr != nullptr)
		{
			if (VideoTrackDescr->Id != -1)
			{
				TSharedRef<FVlcMediaVideoTrack, ESPMode::ThreadSafe> NewTrack = MakeShareable(
					new FVlcMediaVideoTrack(Player, VideoTrackDescr)
					);

				Tracks.Add(NewTrack);

				// hack
				CurrentVideoTrack = NewTrack;
			}

			VideoTrackDescr = VideoTrackDescr->Next;
		}
	}
	FVlc::TrackDescriptionListRelease(VideoTrackDescr);

	TracksInitialized = true;
}


void FVlcMediaVideoHandler::Shutdown()
{
	if (Player != nullptr)
	{
		FVlc::VideoSetFormatCallbacks(Player, nullptr, nullptr);
		FVlc::VideoSetCallbacks(Player, nullptr, nullptr, nullptr, nullptr);
	}

	CurrentVideoTrack.Reset();
	Tracks.Reset();
}


/* FVlcMediaVideoTrack static functions
*****************************************************************************/

void FVlcMediaVideoHandler::HandleVideoDisplay(void* Opaque, void* /*Picture*/)
{
	auto Handler = (FVlcMediaVideoHandler*)Opaque;

	if (Handler != nullptr)
	{
		auto CurrentVideoTrack = Handler->CurrentVideoTrack.Pin();

		if (CurrentVideoTrack.IsValid())
		{
			CurrentVideoTrack->SetTime(Handler->CurrentTime);
			CurrentVideoTrack->ProcessFrameBuffer();
		}
	}
}


void* FVlcMediaVideoHandler::HandleVideoLock(void* Opaque, void** Planes)
{
	auto Handler = (FVlcMediaVideoHandler*)Opaque;

	if (Handler != nullptr)
	{
		auto CurrentVideoTrack = Handler->CurrentVideoTrack.Pin();

		if (CurrentVideoTrack.IsValid())
		{
			*Planes = CurrentVideoTrack->GetFrameData();
		}
	}

	return nullptr;
}


void FVlcMediaVideoHandler::HandleVideoUnlock(void* Opaque, void* /*Picture*/, void* const* Planes)
{
}
