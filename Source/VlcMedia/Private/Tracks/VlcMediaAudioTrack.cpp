// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"


/* FVlcMediaAudioTrack structors
 *****************************************************************************/

FVlcMediaAudioTrack::FVlcMediaAudioTrack(FLibvlcMediaPlayer* InPlayer, uint32 InTrackIndex, FLibvlcTrackDescription* Descr)
	: FVlcMediaTrack(InPlayer, InTrackIndex, Descr)
	, AudioTrackId(Descr->Id)
	, NumChannels(0)
	, SamplesPerSecond(0)
{
	FVlc::AudioSetFormatCallbacks(
		InPlayer,
		&FVlcMediaAudioTrack::HandleAudioSetup,
		&FVlcMediaAudioTrack::HandleAudioCleanup
	);

	FVlc::AudioSetCallbacks(
		InPlayer,
		&FVlcMediaAudioTrack::HandleAudioPlay,
		&FVlcMediaAudioTrack::HandleAudioPause,
		&FVlcMediaAudioTrack::HandleAudioResume,
		&FVlcMediaAudioTrack::HandleAudioFlush,
		&FVlcMediaAudioTrack::HandleAudioDrain,
		this
	);
}


FVlcMediaAudioTrack::~FVlcMediaAudioTrack()
{
	//FVlc::AudioSetCallbacks(GetPlayer(), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	//FVlc::AudioSetFormatCallbacks(GetPlayer(), nullptr, nullptr);
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
	return (!IsEnabled() || (FVlc::AudioSetTrack(GetPlayer(), -1) == 0));
}


bool FVlcMediaAudioTrack::Enable()
{
	// @todo gmp: implement support for multiple active VLC tracks
	return (FVlc::AudioSetTrack(GetPlayer(), AudioTrackId) == 0);
}


bool FVlcMediaAudioTrack::IsEnabled() const
{
	// @todo gmp: implement support for multiple active VLC tracks
	return (FVlc::AudioGetTrack(GetPlayer()) == AudioTrackId);
}


/* FVlcMediaAudioTrack static functions
*****************************************************************************/

void FVlcMediaAudioTrack::HandleAudioDrain(void* Opaque)
{
}


void FVlcMediaAudioTrack::HandleAudioFlush(void* Opaque, int64 Timestamp)
{
}


void FVlcMediaAudioTrack::HandleAudioPause(void* Opaque, int64 Timestamp)
{
}


void FVlcMediaAudioTrack::HandleAudioPlay(void* Opaque, void* Samples, uint32 Count, int64 Timestamp)
{
	if (Opaque != nullptr)
	{
		FVlcMediaAudioTrack* AudioTrack = (FVlcMediaAudioTrack*)Opaque;
		AudioTrack->ProcessMediaSample(Samples, Count * AudioTrack->NumChannels * sizeof(int16), 0.0f);
	}
}


void FVlcMediaAudioTrack::HandleAudioResume(void* Opaque, int64 Timestamp)
{
}


int FVlcMediaAudioTrack::HandleAudioSetup(void** Opaque, ANSICHAR* Format, uint32* Rate, uint32* Channels)
{
	if (Opaque == nullptr)
	{
		return -1;
	}

	Format = "S16N"; // force sample format to 16-bit signed integer

	FVlcMediaAudioTrack* AudioTrack = (FVlcMediaAudioTrack*)*Opaque;
	{
		// @todo gmp: make audio specs configurable
		AudioTrack->NumChannels = *Channels;
		AudioTrack->SamplesPerSecond = *Rate;
	}

	return 0;
}


void FVlcMediaAudioTrack::HandleAudioCleanup(void* Opaque)
{
}
