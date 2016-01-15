// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"


#define LOCTEXT_NAMESPACE "FVlcMediaTrack"


/* FVlcMediaTrack structors
 *****************************************************************************/

FVlcMediaTrack::FVlcMediaTrack(FLibvlcMediaPlayer* InPlayer, FLibvlcTrackDescription* Descr)
    : LastTime(FTimespan::Zero())
	, Name(ANSI_TO_TCHAR(Descr->Name))
	, Player(InPlayer)
{
	if (Name.IsEmpty())
	{
		DisplayName = FText::Format(LOCTEXT("UnnamedTrackFormat", "Unnamed Track {0}"), FText::AsNumber((uint32)Descr->Id));
	}
	else
	{
		DisplayName = FText::FromString(Name);
	}
}


/* IMediaStream interface
 *****************************************************************************/

void FVlcMediaTrack::AddSink(const IMediaSinkRef& Sink)
{
	FScopeLock Lock(&SinksLock);
	Sinks.AddUnique(IMediaSinkWeakPtr(Sink));
}


FText FVlcMediaTrack::GetDisplayName() const
{
	return DisplayName;
}


FString FVlcMediaTrack::GetLanguage() const
{
	return FString();
}


FString FVlcMediaTrack::GetName() const
{
	return Name;
}


bool FVlcMediaTrack::IsMutuallyExclusive(const IMediaStreamRef& Other) const
{
	// for now we only allow one type of track at a time
	// @todo gmp: implement support for multiple active VLC tracks
	return (&Other.Get() != this);
}


bool FVlcMediaTrack::IsProtected() const
{
	return false;
}


void FVlcMediaTrack::RemoveSink(const IMediaSinkRef& Sink)
{
	FScopeLock Lock(&SinksLock);
	Sinks.RemoveSingle(IMediaSinkWeakPtr(Sink));
}


/* FVlcMediaTrack implementation
 *****************************************************************************/

void FVlcMediaTrack::ProcessMediaSample(const void* SampleBuffer, uint32 SampleSize, FTimespan SampleDuration)
{
	FScopeLock Lock(&SinksLock);

	for (IMediaSinkWeakPtr& SinkPtr : Sinks)
	{
		IMediaSinkPtr Sink = SinkPtr.Pin();

		if (Sink.IsValid())
		{
			Sink->ProcessMediaSample(SampleBuffer, SampleSize, SampleDuration, LastTime);
		}
	}
}


#undef LOCTEXT_NAMESPACE
