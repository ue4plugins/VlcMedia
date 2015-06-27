// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"


/* FVlcMediaVideoTrack structors
 *****************************************************************************/

FVlcMediaVideoTrack::FVlcMediaVideoTrack(FLibvlcMediaPlayer* InPlayer, uint32 InTrackIndex, FLibvlcTrackDescription* Descr)
	: FVlcMediaTrack(InPlayer, InTrackIndex, Descr)
	, Dimensions(ForceInitToZero)
	, LastDelta(FTimespan::Zero())
	, VideoTrackId(Descr->Id)
{
	Dimensions.X = FVlc::VideoGetWidth(InPlayer);
	Dimensions.Y = FVlc::VideoGetHeight(InPlayer);

	if (Dimensions.GetMin() > 0)
	{
		FrameBuffer.AddUninitialized(Dimensions.X * Dimensions.Y * 4);

		// @todo gmp: implement support for multiple active VLC tracks
		FVlc::VideoSetCallbacks(
			InPlayer,
			&FVlcMediaVideoTrack::HandleVideoLock,
			&FVlcMediaVideoTrack::HandleVideoUnlock,
			&FVlcMediaVideoTrack::HandleVideoDisplay,
			this);

		FVlc::VideoSetFormat(InPlayer, "RV32", Dimensions.X, Dimensions.Y, Dimensions.X * 4);
	}
}


FVlcMediaVideoTrack::~FVlcMediaVideoTrack()
{
	if (Dimensions.GetMin() > 0)
	{
		// @todo gmp: this is probably not thread-safe
		//FVlc::VideoSetCallbacks(GetPlayer(), nullptr, nullptr, nullptr, nullptr);
	}
}


/* IMediaTrack interface
 *****************************************************************************/

bool FVlcMediaVideoTrack::Disable()
{
	return (!IsEnabled() || (FVlc::VideoSetTrack(GetPlayer(), -1) == 0));
}


bool FVlcMediaVideoTrack::Enable()
{
	// @todo gmp: implement support for multiple active VLC tracks
	return (FVlc::VideoSetTrack(GetPlayer(), VideoTrackId) == 0);
}


const IMediaTrackAudioDetails& FVlcMediaVideoTrack::GetAudioDetails() const
{
	check(false); // not a caption track
	return (IMediaTrackAudioDetails&)*this;
}


const IMediaTrackCaptionDetails& FVlcMediaVideoTrack::GetCaptionDetails() const
{
	check(false); // not a caption track
	return (IMediaTrackCaptionDetails&)*this;
}


EMediaTrackTypes FVlcMediaVideoTrack::GetType() const
{
	return EMediaTrackTypes::Video;
}


const IMediaTrackVideoDetails& FVlcMediaVideoTrack::GetVideoDetails() const
{
	return *this;
}


bool FVlcMediaVideoTrack::IsEnabled() const
{
	// @todo gmp: implement support for multiple active VLC tracks
	return (FVlc::VideoGetTrack(GetPlayer()) == VideoTrackId);
}


/* IMediaTrackVideoDetails interface
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


/* FVlcMediaVideoTrack static functions
 *****************************************************************************/

void* FVlcMediaVideoTrack::HandleVideoLock(void* Opaque, void** Planes)
{
	if (Opaque != nullptr)
	{
		FVlcMediaVideoTrack* VideoTrack = (FVlcMediaVideoTrack*)Opaque;
		{
			*Planes = VideoTrack->FrameBuffer.GetData();
		}
	}

	return nullptr;
}


void FVlcMediaVideoTrack::HandleVideoUnlock(void* Opaque, void* /*Picture*/, void* const* Planes)
{
}


void FVlcMediaVideoTrack::HandleVideoDisplay(void* Opaque, void* /*Picture*/)
{
	if (Opaque != nullptr)
	{
		FVlcMediaVideoTrack* VideoTrack = (FVlcMediaVideoTrack*)Opaque;
		{
			VideoTrack->ProcessMediaSample(VideoTrack->FrameBuffer.GetData(), VideoTrack->FrameBuffer.Num(), 0.0f);
		}
	}
}
