// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"


#define LOCTEXT_NAMESPACE "FVlcMediaTrack"


/* FVlcMediaTrack structors
 *****************************************************************************/

FVlcMediaTrack::FVlcMediaTrack(FLibvlcMediaPlayer* InPlayer, uint32 InTrackIndex, FLibvlcTrackDescription* Descr)
    : LastTime(0.0f)
	, Name(ANSI_TO_TCHAR(Descr->Name))
	, Player(InPlayer)
	, TrackIndex(InTrackIndex)
{
	if (Name.IsEmpty())
	{
		DisplayName = FText::Format(LOCTEXT("UnnamedTrackFormat", "Unnamed Track {0}"), FText::AsNumber((uint32)TrackIndex));
	}
	else
	{
		DisplayName = FText::FromString(Name);
	}
}


/* IMediaTrack interface
 *****************************************************************************/

void FVlcMediaTrack::AddSink(const IMediaSinkRef& Sink)
{
	Sinks.AddUnique(IMediaSinkWeakPtr(Sink));
}


FText FVlcMediaTrack::GetDisplayName() const
{
	return DisplayName;
}


uint32 FVlcMediaTrack::GetIndex() const
{
	return TrackIndex;
}


FString FVlcMediaTrack::GetLanguage() const
{
	return FString();
}


FString FVlcMediaTrack::GetName() const
{
	return Name;
}


bool FVlcMediaTrack::IsMutuallyExclusive(const IMediaTrackRef& Other) const
{
	// for now we only allow one type of track at a time
	// @todo gmp: implement support for multiple active VLC tracks
	return ((GetIndex() != Other->GetIndex()) && (GetType() == Other->GetType()));
}


bool FVlcMediaTrack::IsProtected() const
{
	return false;
}


void FVlcMediaTrack::RemoveSink(const IMediaSinkRef& Sink)
{
	Sinks.RemoveSingle(IMediaSinkWeakPtr(Sink));
}


/* FVlcMediaTrack implementation
 *****************************************************************************/

void FVlcMediaTrack::ProcessMediaSample(const void* SampleBuffer, uint32 SampleSize, FTimespan SampleDuration)
{
	for (IMediaSinkWeakPtr& SinkPtr : Sinks)
	{
		IMediaSinkPtr Sink = SinkPtr.Pin();

		if (Sink.IsValid())
		{
			Sink->ProcessMediaSample(SampleBuffer, SampleSize, SampleDuration, FTimespan::FromSeconds(LastTime));
		}
	}
}


#undef LOCTEXT_NAMESPACE
