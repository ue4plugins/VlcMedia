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

bool FVlcMediaTracks::GetAudioTrackFormat(int32 TrackIndex, int32 FormatIndex, FMediaAudioTrackFormat& OutFormat) const
{
	if (!AudioTracks.IsValidIndex(TrackIndex) || (FormatIndex != 0))
	{
		return false;
	}

	// @todo gmp: fix audio format
	OutFormat.BitsPerSample = 0;
	OutFormat.NumChannels = 2;
	OutFormat.SampleRate = 44100;
	OutFormat.TypeName = TEXT("PCM");

	return true;
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


int32 FVlcMediaTracks::GetNumTrackFormats(EMediaTrackType TrackType, int32 TrackIndex) const
{
	return ((TrackIndex >= 0) && (TrackIndex < GetNumTracks(TrackType))) ? 1 : 0;
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


int32 FVlcMediaTracks::GetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex) const
{
	return (GetSelectedTrack(TrackType) != INDEX_NONE) ? 0 : INDEX_NONE;
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


bool FVlcMediaTracks::GetVideoTrackFormat(int32 TrackIndex, int32 FormatIndex, FMediaVideoTrackFormat& OutFormat) const
{
	if (!VideoTracks.IsValidIndex(TrackIndex) || (FormatIndex != 0) || (Player == nullptr))
	{
		return false;
	}

	// @todo gmp: fix video specs
	OutFormat.Dim = FIntPoint(FVlc::VideoGetWidth(Player), FVlc::VideoGetHeight(Player));
	OutFormat.FrameRate = FVlc::MediaPlayerGetFps(Player);
	OutFormat.FrameRates = TRange<float>(OutFormat.FrameRate);
	OutFormat.TypeName = TEXT("Default");

	return true;
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


bool FVlcMediaTracks::SetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex, int32 FormatIndex)
{
	if ((Player == nullptr) || (FormatIndex != 0))
	{
		return false;
	}

	switch (TrackType)
	{
	case EMediaTrackType::Audio:
		return AudioTracks.IsValidIndex(TrackIndex);

	case EMediaTrackType::Caption:
		return CaptionTracks.IsValidIndex(TrackIndex);

	case EMediaTrackType::Video:
		return VideoTracks.IsValidIndex(TrackIndex);

	default:
		return false;
	}
}


#undef LOCTEXT_NAMESPACE
