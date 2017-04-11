// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VlcMediaTracks.h"
#include "Vlc.h"


#define LOCTEXT_NAMESPACE "FVlcMediaTracks"


/* FVlcMediaTracks structors
*****************************************************************************/

FVlcMediaTracks::FVlcMediaTracks()
	: Player(nullptr)
{ }


/* FVlcMediaTracks interface
*****************************************************************************/

void FVlcMediaTracks::Initialize(FLibvlcMediaPlayer& InPlayer, FString& OutInfo)
{
	Shutdown();

	Player = &InPlayer;

	int32 Width = FVlc::VideoGetWidth(Player);
	int32 Height = FVlc::VideoGetHeight(Player);
	int32 StreamCount = 0;

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
				FTrack Track;
				{
					Track.Name = ANSI_TO_TCHAR(AudioTrackDescr->Name);
					Track.DisplayName = Track.Name.IsEmpty()
						? FText::Format(LOCTEXT("AudioTrackFormat", "Audio Track {0}"), FText::AsNumber((uint32)AudioTrackDescr->Id))
						: FText::FromString(Track.Name);
				}

				AudioTracks.Add(Track);

				OutInfo += FString::Printf(TEXT("Stream %i\n"), StreamCount);
				OutInfo += TEXT("    Type: Audio\n");
				OutInfo += FString::Printf(TEXT("    Name: %s\n"), *Track.Name);
				OutInfo += TEXT("\n");

				++StreamCount;
			}

			AudioTrackDescr = AudioTrackDescr->Next;
		}
	}
	FVlc::TrackDescriptionListRelease(AudioTrackDescr);

	// initialize caption tracks
	FLibvlcTrackDescription* CaptionTrackDescr = FVlc::VideoGetSpuDescription(Player);
	{
		while (CaptionTrackDescr != nullptr)
		{
			if (CaptionTrackDescr->Id != -1)
			{
				FTrack Track;
				{
					Track.Name = ANSI_TO_TCHAR(CaptionTrackDescr->Name);
					Track.DisplayName = Track.Name.IsEmpty()
						? FText::Format(LOCTEXT("CaptionTrackFormat", "Caption Track {0}"), FText::AsNumber((uint32)CaptionTrackDescr->Id))
						: FText::FromString(Track.Name);
				}

				CaptionTracks.Add(Track);
				
				OutInfo += FString::Printf(TEXT("Stream %i\n"), StreamCount);
				OutInfo += TEXT("    Type: Caption\n");
				OutInfo += FString::Printf(TEXT("    Name: %s\n"), *Track.Name);
				OutInfo += TEXT("\n");

				++StreamCount;
			}

			CaptionTrackDescr = CaptionTrackDescr->Next;
		}
	}
	FVlc::TrackDescriptionListRelease(CaptionTrackDescr);

	// initialize video tracks
	FLibvlcTrackDescription* VideoTrackDescr = FVlc::VideoGetTrackDescription(Player);
	{
		while (VideoTrackDescr != nullptr)
		{
			if (VideoTrackDescr->Id != -1)
			{
				FTrack Track;
				{
					Track.Name = ANSI_TO_TCHAR(VideoTrackDescr->Name);
					Track.DisplayName = Track.Name.IsEmpty()
						? FText::Format(LOCTEXT("VideoTrackFormat", "Video Track {0}"), FText::AsNumber((uint32)VideoTrackDescr->Id))
						: FText::FromString(Track.Name);
				}

				VideoTracks.Add(Track);

				OutInfo += FString::Printf(TEXT("Stream %i\n"), StreamCount);
				OutInfo += TEXT("    Type: Video\n");
				OutInfo += FString::Printf(TEXT("    Name: %s\n"), *Track.Name);
				OutInfo += TEXT("\n");

				++StreamCount;
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


uint32 FVlcMediaTracks::GetAudioTrackSampleRate(int32 TrackIndex) const
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

	case EMediaTrackType::Caption:
		return CaptionTracks.Num();
	
	case EMediaTrackType::Video:
		return VideoTracks.Num();
	
	default:
		return 0;
	}
}


int32 FVlcMediaTracks::GetSelectedTrack(EMediaTrackType TrackType) const
{
	if (Player == nullptr)
	{
		return INDEX_NONE;
	}

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

	case EMediaTrackType::Caption:
		if (CaptionTracks.IsValidIndex(TrackIndex))
		{
			return CaptionTracks[TrackIndex].DisplayName;
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
	return TEXT("und"); // libvlc currently doesn't provide language codes
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

	case EMediaTrackType::Caption:
		if (CaptionTracks.IsValidIndex(TrackIndex))
		{
			return CaptionTracks[TrackIndex].Name;
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
	return (Player != nullptr) ? FVlc::MediaPlayerGetFps(Player) : 0;
}


bool FVlcMediaTracks::SelectTrack(EMediaTrackType TrackType, int32 TrackIndex)
{
	if (Player == nullptr)
	{
		return false;
	}

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
