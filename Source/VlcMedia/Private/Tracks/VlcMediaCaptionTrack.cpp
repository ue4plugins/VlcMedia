// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"


/* FVlcMediaCaptionTrack structors
 *****************************************************************************/

FVlcMediaCaptionTrack::FVlcMediaCaptionTrack(FLibvlcMediaPlayer* InPlayer, uint32 InTrackIndex, FLibvlcTrackDescription* Descr)
	: FVlcMediaTrack(InPlayer, InTrackIndex, Descr)
	, SpuId(Descr->Id)
{ }


/* IMediaTrack interface
 *****************************************************************************/

bool FVlcMediaCaptionTrack::Disable()
{
	return true;
}


bool FVlcMediaCaptionTrack::Enable()
{
	// @todo gmp: implement support for multiple active VLC tracks
	return (FVlc::VideoSetSpu(GetPlayer(), SpuId) == 0);
}


const IMediaTrackAudioDetails& FVlcMediaCaptionTrack::GetAudioDetails() const
{
	check(false); // not an audio track
	return (IMediaTrackAudioDetails&)*this;
}


const IMediaTrackCaptionDetails& FVlcMediaCaptionTrack::GetCaptionDetails() const
{
	return *this;
}


EMediaTrackTypes FVlcMediaCaptionTrack::GetType() const
{
	return EMediaTrackTypes::Caption;
}


const IMediaTrackVideoDetails& FVlcMediaCaptionTrack::GetVideoDetails() const
{
	check(false); // not a video track
	return (IMediaTrackVideoDetails&)*this;
}


bool FVlcMediaCaptionTrack::IsEnabled() const
{
	// @todo gmp: implement support for multiple active VLC tracks
	return (FVlc::VideoGetSpu(GetPlayer()) == SpuId);
}
