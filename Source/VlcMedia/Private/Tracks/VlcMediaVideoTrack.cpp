// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"


/* FVlcMediaVideoTrack structors
 *****************************************************************************/

FVlcMediaVideoTrack::FVlcMediaVideoTrack(FLibvlcMediaPlayer* InPlayer, FLibvlcTrackDescription* Descr)
	: FVlcMediaTrack(InPlayer, Descr)
//	, Dimensions(ForceInitToZero)
	, LastDelta(FTimespan::Zero())
	, VideoTrackId(Descr->Id)
{
	// @todo gmp: fix me: dimensions should be initialized async in setup callback
	Dimensions.X = FVlc::VideoGetWidth(InPlayer);
	Dimensions.Y = FVlc::VideoGetHeight(InPlayer);
	FrameBuffer.AddUninitialized(Dimensions.X * Dimensions.Y * 4);

	if (IsEnabled())
	{
		Enable();
	}
}


FVlcMediaVideoTrack::~FVlcMediaVideoTrack()
{
	//Disable();
}


/* IMediaVideoTrack interface
 *****************************************************************************/

uint32 FVlcMediaVideoTrack::GetBitRate() const
{
	return 0;
}


FIntPoint FVlcMediaVideoTrack::GetDimensions() const
{
	return Dimensions;
}


float FVlcMediaVideoTrack::GetFrameRate() const
{
	return FVlc::MediaPlayerGetFps(GetPlayer());
}


IMediaStream& FVlcMediaVideoTrack::GetStream()
{
	return *this;
}


#if WITH_ENGINE
void FVlcMediaVideoTrack::BindTexture(class FRHITexture* Texture)
{
	// @todo vlc: gmp: implement texture binding
}


void FVlcMediaVideoTrack::UnbindTexture(class FRHITexture* Texture)
{
	// @todo vlc: gmp: implement texture binding
}
#endif


/* IMediaStream interface
 *****************************************************************************/

bool FVlcMediaVideoTrack::Disable()
{
	if (!IsEnabled())
	{
		return true;
	}

	return (FVlc::VideoSetTrack(GetPlayer(), -1) == 0);
}


bool FVlcMediaVideoTrack::Enable()
{
	return (FVlc::VideoSetTrack(GetPlayer(), VideoTrackId) == 0);
}


bool FVlcMediaVideoTrack::IsEnabled() const
{
	return (FVlc::VideoGetTrack(GetPlayer()) == VideoTrackId);
}
