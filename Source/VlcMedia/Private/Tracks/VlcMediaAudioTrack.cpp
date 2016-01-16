// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"


/* FVlcMediaAudioTrack structors
 *****************************************************************************/

FVlcMediaAudioTrack::FVlcMediaAudioTrack(FLibvlcMediaPlayer* InPlayer, FLibvlcTrackDescription* Descr)
	: FVlcMediaTrack(InPlayer, Descr)
	, AudioTrackId(Descr->Id)
	, NumChannels(2) // @todo gmp: fix audio specs
	, SamplesPerSecond(44100) // @todo gmp: fix audio specs
{
	if (IsEnabled())
	{
		Enable();
	}
}


FVlcMediaAudioTrack::~FVlcMediaAudioTrack()
{
	//Disable();
}


/* IMediaAudioTrack interface
 *****************************************************************************/

uint32 FVlcMediaAudioTrack::GetNumChannels() const
{
	return NumChannels;
}


uint32 FVlcMediaAudioTrack::GetSamplesPerSecond() const
{
	return SamplesPerSecond;
}


IMediaStream& FVlcMediaAudioTrack::GetStream()
{
	return *this;
}


/* IMediaStream interface
 *****************************************************************************/

bool FVlcMediaAudioTrack::Disable()
{
	if (!IsEnabled())
	{
		return true;
	}

	return (FVlc::AudioSetTrack(GetPlayer(), -1) == 0);
}


bool FVlcMediaAudioTrack::Enable()
{
	return (FVlc::AudioSetTrack(GetPlayer(), AudioTrackId) == 0);
}


bool FVlcMediaAudioTrack::IsEnabled() const
{
	return (FVlc::AudioGetTrack(GetPlayer()) == AudioTrackId);
}
