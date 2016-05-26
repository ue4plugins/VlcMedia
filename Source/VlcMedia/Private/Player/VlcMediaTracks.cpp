// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPCH.h"
#include "Vlc.h"
#include "VlcMediaTracks.h"


#define LOCTEXT_NAMESPACE "FVlcMediaTracks"


/* FVlcMediaTracks structors
*****************************************************************************/

FVlcMediaTracks::FVlcMediaTracks()
	: Player(nullptr)
{ }


/* FVlcMediaTracks interface
*****************************************************************************/

void FVlcMediaTracks::Initialize(FLibvlcMediaPlayer& InPlayer)
{
	Shutdown();

	Player = &InPlayer;

	int32 Width = FVlc::VideoGetWidth(Player);
	int32 Height = FVlc::VideoGetHeight(Player);

	// @todo gmp: fix audio specs
	FVlc::AudioSetFormat(Player, "S16N", 44100, 2);
	FVlc::VideoSetFormat(Player, "RV32", Width, Height, Width * 4);

	// initialize audio tracks
	FLibvlcTrackDescription* AudioTrackDescr = FVlc::AudioGetTrackDescription(Player);
	{
		while (AudioTrackDescr != nullptr)
		{
			if (AudioTrackDescr->Id != -1)
			{
				FVlcMediaTrack Track;
				{
					Track.Name = ANSI_TO_TCHAR(AudioTrackDescr->Name);
					Track.DisplayName = Track.Name.IsEmpty()
						? FText::Format(LOCTEXT("AudioTrackFormat", "Audio Track {0}"), FText::AsNumber((uint32)AudioTrackDescr->Id))
						: FText::FromString(Track.Name);
				}

				AudioTracks.Add(Track);
			}

			AudioTrackDescr = AudioTrackDescr->Next;
		}
	}
	FVlc::TrackDescriptionListRelease(AudioTrackDescr);

	// initialize video tracks
	FLibvlcTrackDescription* VideoTrackDescr = FVlc::VideoGetTrackDescription(Player);
	{
		while (VideoTrackDescr != nullptr)
		{
			if (VideoTrackDescr->Id != -1)
			{
				FVlcMediaTrack Track;
				{
					Track.Name = ANSI_TO_TCHAR(VideoTrackDescr->Name);
					Track.DisplayName = Track.Name.IsEmpty()
						? FText::Format(LOCTEXT("VideoTrackFormat", "Video Track {0}"), FText::AsNumber((uint32)VideoTrackDescr->Id))
						: FText::FromString(Track.Name);
				}

				VideoTracks.Add(Track);
			}

			VideoTrackDescr = VideoTrackDescr->Next;
		}
	}
	FVlc::TrackDescriptionListRelease(VideoTrackDescr);
}


void FVlcMediaTracks::Shutdown()
{
	if (Player != nullptr)
	{
		AudioTracks.Reset();
		VideoTracks.Reset();
		Player = nullptr;
	}
}


/* IMediaTracks interface
*****************************************************************************/

uint32 FVlcMediaTracks::GetAudioTrackChannels(int32 TrackIndex) const
{
	// @todo gmp: fix audio specs
	return 2;
}


uint32 FVlcMediaTracks::GetAudioTrackSamplesPerSeconds(int32 TrackIndex) const
{
	// @todo gmp: fix audio specs
	return 44100;
}


int32 FVlcMediaTracks::GetNumTracks(EMediaTrackType TrackType) const
{
	switch (TrackType)
	{
	case EMediaTrackType::Audio:
		return AudioTracks.Num();
	case EMediaTrackType::Video:
		return VideoTracks.Num();
	default:
		return 0;
	}
}


int32 FVlcMediaTracks::GetSelectedTrack(EMediaTrackType TrackType) const
{
	switch (TrackType)
	{
	case EMediaTrackType::Audio:
		return FVlc::AudioGetTrack(Player);

	case EMediaTrackType::Caption:
		return FVlc::VideoGetSpu(Player);

	case EMediaTrackType::Video:
		return FVlc::VideoGetTrack(Player);

	default:
		return INDEX_NONE;
	}
}


FText FVlcMediaTracks::GetTrackDisplayName(EMediaTrackType TrackType, int32 TrackIndex) const
{
	switch (TrackType)
	{
	case EMediaTrackType::Audio:
		if (AudioTracks.IsValidIndex(TrackIndex))
		{
			return AudioTracks[TrackIndex].DisplayName;
		}
		break;

	case EMediaTrackType::Video:
		if (VideoTracks.IsValidIndex(TrackIndex))
		{
			return VideoTracks[TrackIndex].DisplayName;
		}

	default:
		break;
	}

	return FText::GetEmpty();
}


FString FVlcMediaTracks::GetTrackLanguage(EMediaTrackType TrackType, int32 TrackIndex) const
{
	return TEXT("und");
}


FString FVlcMediaTracks::GetTrackName(EMediaTrackType TrackType, int32 TrackIndex) const
{
	switch (TrackType)
	{
	case EMediaTrackType::Audio:
		if (AudioTracks.IsValidIndex(TrackIndex))
		{
			return AudioTracks[TrackIndex].Name;
		}
		break;

	case EMediaTrackType::Video:
		if (VideoTracks.IsValidIndex(TrackIndex))
		{
			return VideoTracks[TrackIndex].Name;
		}

	default:
		break;
	}

	return FString();
}


uint32 FVlcMediaTracks::GetVideoTrackBitRate(int32 TrackIndex) const
{
	return 0;
}


FIntPoint FVlcMediaTracks::GetVideoTrackDimensions(int32 TrackIndex) const
{
	return (Player != nullptr)
		? FIntPoint(FVlc::VideoGetWidth(Player), FVlc::VideoGetHeight(Player))
		: FIntPoint(0, 0);
}


float FVlcMediaTracks::GetVideoTrackFrameRate(int32 TrackIndex) const
{
	return FVlc::MediaPlayerGetFps(Player);
}


bool FVlcMediaTracks::SelectTrack(EMediaTrackType TrackType, int32 TrackIndex)
{
	switch (TrackType)
	{
	case EMediaTrackType::Audio:
		return (FVlc::AudioSetTrack(Player, TrackIndex) == 0);

	case EMediaTrackType::Caption:
		return (FVlc::VideoSetSpu(Player, TrackIndex) == 0);

	case EMediaTrackType::Video:
		return (FVlc::VideoSetTrack(Player, TrackIndex) == 0);

	default:
		return false;
	}
}


#undef LOCTEXT_NAMESPACE
