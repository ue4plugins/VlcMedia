// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"


/* FVlcMediaAudioTrack structors
 *****************************************************************************/

FVlcMediaAudioTrack::FVlcMediaAudioTrack(FLibvlcMediaPlayer* InPlayer, uint32 InTrackIndex, FLibvlcTrackDescription* Descr)
	: FVlcMediaTrack(InPlayer, InTrackIndex, Descr)
	, AudioTrackId(Descr->Id)
{ }


/* IMediaAudioTrack interface
 *****************************************************************************/

uint32 FVlcMediaAudioTrack::GetNumChannels() const
{
	return 1;
}


uint32 FVlcMediaAudioTrack::GetSamplesPerSecond() const
{
	return 0;
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
