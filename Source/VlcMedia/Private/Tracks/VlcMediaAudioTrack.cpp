// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"


/* FVlcMediaAudioTrack structors
 *****************************************************************************/

FVlcMediaAudioTrack::FVlcMediaAudioTrack(FLibvlcMediaPlayer* InPlayer, FLibvlcTrackDescription* Descr)
	: FVlcMediaTrack(InPlayer, Descr)
	, AudioTrackId(Descr->Id)
	, NumChannels(0)
	, SamplesPerSecond(0)
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

	FVlc::AudioSetCallbacks(GetPlayer(), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	FVlc::AudioSetFormatCallbacks(GetPlayer(), nullptr, nullptr);

	return (FVlc::AudioSetTrack(GetPlayer(), -1) == 0);
}


bool FVlcMediaAudioTrack::Enable()
{
	FVlc::AudioSetCallbacks(
		GetPlayer(),
		&FVlcMediaAudioTrack::HandleAudioPlay,
		&FVlcMediaAudioTrack::HandleAudioPause,
		&FVlcMediaAudioTrack::HandleAudioResume,
		&FVlcMediaAudioTrack::HandleAudioFlush,
		&FVlcMediaAudioTrack::HandleAudioDrain,
		this
	);

	FVlc::AudioSetFormatCallbacks(
		GetPlayer(),
		&FVlcMediaAudioTrack::HandleAudioSetup,
		&FVlcMediaAudioTrack::HandleAudioCleanup
	);

	return (FVlc::AudioSetTrack(GetPlayer(), AudioTrackId) == 0);
}


bool FVlcMediaAudioTrack::IsEnabled() const
{
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
	auto AudioTrack = (FVlcMediaAudioTrack*)Opaque;

	if (AudioTrack != nullptr)
	{
		AudioTrack->ProcessMediaSample(Samples, Count * AudioTrack->NumChannels * sizeof(int16), 0.0f);
	}
}


void FVlcMediaAudioTrack::HandleAudioResume(void* Opaque, int64 Timestamp)
{
}


int FVlcMediaAudioTrack::HandleAudioSetup(void** Opaque, ANSICHAR* Format, uint32* Rate, uint32* Channels)
{
	auto AudioTrack = (FVlcMediaAudioTrack*)*Opaque;

	if (AudioTrack == nullptr)
	{
		return -1;
	}

	FMemory::Memcpy(Format, "S16N", 4); // force sample format to 16-bit signed integer

	// @todo gmp: make audio specs configurable?
	AudioTrack->NumChannels = *Channels;
	AudioTrack->SamplesPerSecond = *Rate;

	return 0;
}


void FVlcMediaAudioTrack::HandleAudioCleanup(void* Opaque)
{
}
