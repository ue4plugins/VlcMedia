// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"


/* FVlcMediaCaptionTrack structors
 *****************************************************************************/

FVlcMediaCaptionTrack::FVlcMediaCaptionTrack(FLibvlcMediaPlayer* InPlayer, FLibvlcTrackDescription* Descr)
	: FVlcMediaTrack(InPlayer, Descr)
	, SpuId(Descr->Id)
{ }


/* IMediaCaptionTrack interface
 *****************************************************************************/

IMediaStream& FVlcMediaCaptionTrack::GetStream()
{
	return *this;
}


/* IMediaStream interface
 *****************************************************************************/

bool FVlcMediaCaptionTrack::Disable()
{
	return (!IsEnabled() || (FVlc::VideoSetSpu(GetPlayer(), -1) == 0));
}


bool FVlcMediaCaptionTrack::Enable()
{
	// @todo gmp: implement support for multiple active VLC tracks
	return (FVlc::VideoSetSpu(GetPlayer(), SpuId) == 0);
}


bool FVlcMediaCaptionTrack::IsEnabled() const
{
	// @todo gmp: implement support for multiple active VLC tracks
	return (FVlc::VideoGetSpu(GetPlayer()) == SpuId);
}
