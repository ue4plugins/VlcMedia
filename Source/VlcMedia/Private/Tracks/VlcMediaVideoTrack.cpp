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

	FVlc::VideoSetCallbacks(GetPlayer(), nullptr, nullptr, nullptr, nullptr);
	FVlc::VideoSetFormatCallbacks(GetPlayer(), nullptr, nullptr);

	return (FVlc::VideoSetTrack(GetPlayer(), -1) == 0);
}


bool FVlcMediaVideoTrack::Enable()
{
	FVlc::VideoSetCallbacks(
		GetPlayer(),
		&FVlcMediaVideoTrack::HandleVideoLock,
		&FVlcMediaVideoTrack::HandleVideoUnlock,
		&FVlcMediaVideoTrack::HandleVideoDisplay,
		this
	);

	FVlc::VideoSetFormatCallbacks(
		GetPlayer(),
		&FVlcMediaVideoTrack::HandleVideoSetup,
		&FVlcMediaVideoTrack::HandleVideoCleanup
	);

	return (FVlc::VideoSetTrack(GetPlayer(), VideoTrackId) == 0);
}


bool FVlcMediaVideoTrack::IsEnabled() const
{
	return (FVlc::VideoGetTrack(GetPlayer()) == VideoTrackId);
}


/* FVlcMediaVideoTrack static functions
 *****************************************************************************/

void FVlcMediaVideoTrack::HandleVideoCleanup(void* Opaque)
{
	auto VideoTrack = (FVlcMediaVideoTrack*)Opaque;

	if (VideoTrack != nullptr)
	{
		// @todo gmp: fix me: dimensions should be initialized async in setup callback
		//VideoTrack->FrameBuffer.Empty();
	}
}


void FVlcMediaVideoTrack::HandleVideoDisplay(void* Opaque, void* /*Picture*/)
{
	auto VideoTrack = (FVlcMediaVideoTrack*)Opaque;

	if (VideoTrack != nullptr)
	{
		VideoTrack->ProcessMediaSample(VideoTrack->FrameBuffer.GetData(), VideoTrack->FrameBuffer.Num(), 0.0f);
	}
}


void* FVlcMediaVideoTrack::HandleVideoLock(void* Opaque, void** Planes)
{
	auto VideoTrack = (FVlcMediaVideoTrack*)Opaque;

	if (VideoTrack != nullptr)
	{
		*Planes = VideoTrack->FrameBuffer.GetData();
	}

	return nullptr;
}


uint32 FVlcMediaVideoTrack::HandleVideoSetup(void** Opaque, ANSICHAR* Chroma, uint32* Width, uint32* Height, uint32* Pitches, uint32* Lines)
{
	auto VideoTrack = (FVlcMediaVideoTrack*)*Opaque;

	if (VideoTrack == nullptr)
	{
		return 0;
	}

	FMemory::Memcpy(Chroma, "RV32", 4); // force pixel format to RGBA
	*Pitches = *Width * 4;
	*Lines = *Height;

	// @todo gmp: fix me: dimensions should be initialized async in setup callback
/* 		VideoTrack->Dimensions.X = *Width;
	VideoTrack->Dimensions.Y = *Height;

	int32 BufferSize = *Width * *Height * 4;

	VideoTrack->FrameBuffer.Reset(BufferSize);
	VideoTrack->FrameBuffer.AddUninitialized(BufferSize);*/

	return 1;
}


void FVlcMediaVideoTrack::HandleVideoUnlock(void* Opaque, void* /*Picture*/, void* const* Planes)
{
}
