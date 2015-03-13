// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"


/* FVlcMediaPlayer structors
 *****************************************************************************/

FVlcMediaPlayer::FVlcMediaPlayer()
{ }


FVlcMediaPlayer::~FVlcMediaPlayer()
{
	Close();
}


/* IMediaInfo interface
 *****************************************************************************/

FTimespan FVlcMediaPlayer::GetDuration() const
{
	return FTimespan::Zero();
}


TRange<float> FVlcMediaPlayer::GetSupportedRates( EMediaPlaybackDirections Direction, bool Unthinned ) const
{
	return TRange<float>(0.0f);
}


FString FVlcMediaPlayer::GetUrl() const
{
	return FString();
}


bool FVlcMediaPlayer::SupportsRate( float Rate, bool Unthinned ) const
{
	return false;
}


bool FVlcMediaPlayer::SupportsScrubbing() const
{
	return false;
}


bool FVlcMediaPlayer::SupportsSeeking() const
{
	return false;
}


/* IMediaPlayer interface
 *****************************************************************************/

void FVlcMediaPlayer::Close()
{
}


const IMediaInfo& FVlcMediaPlayer::GetMediaInfo() const 
{
	return *this;
}


float FVlcMediaPlayer::GetRate() const
{
	return 0.0f;
}


FTimespan FVlcMediaPlayer::GetTime() const 
{
	return FTimespan::Zero();
}


const TArray<IMediaTrackRef>& FVlcMediaPlayer::GetTracks() const
{
	return Tracks;
}


bool FVlcMediaPlayer::IsLooping() const 
{
	return false;
}


bool FVlcMediaPlayer::IsPaused() const
{
	return false;
}


bool FVlcMediaPlayer::IsPlaying() const
{
	return false;
}


bool FVlcMediaPlayer::IsReady() const
{
	return false;
}


bool FVlcMediaPlayer::Open( const FString& Url )
{
	return false;
}


bool FVlcMediaPlayer::Open( const TSharedRef<TArray<uint8>>& Buffer, const FString& OriginalUrl )
{
	return false;
}


bool FVlcMediaPlayer::Seek( const FTimespan& Time )
{
	return false;
}


bool FVlcMediaPlayer::SetLooping( bool Looping )
{
	return false;
}


bool FVlcMediaPlayer::SetRate( float Rate )
{
	return false;
}
