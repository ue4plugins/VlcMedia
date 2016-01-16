// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"
#include "Ticker.h"
#include "vlc/vlc.h"
#include <mutex>

#define LOCTEXT_NAMESPACE "FVlcMediaPlayer"


/* FVlcMediaPlayer structors
 *****************************************************************************/


FVlcMediaPlayer::FVlcMediaPlayer(FLibvlcInstance* InVlcInstance)
	: CurrentTime(FTimespan::Zero())
	, DesiredRate(0.0)
	, LastPlatformSeconds(0.0)
	, MediaSource(InVlcInstance)
	, Player(nullptr)
	, ShouldLoop(false)
{
	TickerHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FVlcMediaPlayer::HandleTicker), 0.0f);
}


FVlcMediaPlayer::~FVlcMediaPlayer()
{
	Close();

	FTicker::GetCoreTicker().RemoveTicker(TickerHandle);
}


/* IMediaInfo interface
 *****************************************************************************/

FTimespan FVlcMediaPlayer::GetDuration() const
{
	if (Player == nullptr)
	{
		return FTimespan::Zero();
	}

	int64 Length = FVlc::MediaPlayerGetLength(Player);

	if (Length <= 0)
	{
		return GetTime();
	}

	return FTimespan::FromMilliseconds(Length);
}


TRange<float> FVlcMediaPlayer::GetSupportedRates(EMediaPlaybackDirections Direction, bool Unthinned) const
{
	if (Direction == EMediaPlaybackDirections::Reverse)
	{
		return TRange<float>(0.0f);
	}

	return TRange<float>(0.0f, 10.0f);
}


FString FVlcMediaPlayer::GetUrl() const
{
	return MediaSource.GetCurrentUrl();
}


bool FVlcMediaPlayer::SupportsRate(float Rate, bool Unthinned) const
{
	return (Rate >= 0.0f) && (Rate <= 10.f);
}


bool FVlcMediaPlayer::SupportsScrubbing() const
{
	return ((Player != nullptr) && (FVlc::MediaPlayerIsSeekable(Player) != 0));
}


bool FVlcMediaPlayer::SupportsSeeking() const
{
	return ((Player != nullptr) && (FVlc::MediaPlayerIsSeekable(Player) != 0));
}


/* IMediaPlayer interface
 *****************************************************************************/

void FVlcMediaPlayer::Close()
{
	if (Player == nullptr)
	{
		return;
	}

	// detach callback handlers
	AudioHandler.Shutdown();
	VideoHandler.Shutdown();

	// release player
	FVlc::MediaPlayerStop(Player);
	FVlc::MediaPlayerRelease(Player);
	Player = nullptr;

	// reset fields
	CurrentTime = FTimespan::Zero();
	MediaSource.Close();

	MediaEvent.Broadcast(EMediaEvent::TracksChanged);
	MediaEvent.Broadcast(EMediaEvent::MediaClosed);
}


const TArray<IMediaAudioTrackRef>& FVlcMediaPlayer::GetAudioTracks() const
{
	return AudioHandler.GetTracks();
}


const TArray<IMediaCaptionTrackRef>& FVlcMediaPlayer::GetCaptionTracks() const
{
	return CaptionTracks;
}


const IMediaInfo& FVlcMediaPlayer::GetMediaInfo() const 
{
	return *this;
}


float FVlcMediaPlayer::GetRate() const
{
	if ((Player == nullptr) || !IsPlaying())
	{
		return 0.0f;
	}

	return FVlc::MediaPlayerGetRate(Player);
}


FTimespan FVlcMediaPlayer::GetTime() const 
{
	return CurrentTime;
}


const TArray<IMediaVideoTrackRef>& FVlcMediaPlayer::GetVideoTracks() const
{
	return VideoHandler.GetTracks();
}


bool FVlcMediaPlayer::IsLooping() const 
{
	return ShouldLoop;
}


bool FVlcMediaPlayer::IsPaused() const
{
	return ((Player != nullptr) && (FVlc::MediaPlayerGetState(Player) == ELibvlcState::Paused));
}


bool FVlcMediaPlayer::IsPlaying() const
{
	if (Player == nullptr)
	{
		return false;
	}

	ELibvlcState State = FVlc::MediaPlayerGetState(Player);

	return (State == ELibvlcState::Buffering) || (State == ELibvlcState::Playing);
}


bool FVlcMediaPlayer::IsReady() const
{
	if (Player == nullptr)
	{
		return false;
	}

	ELibvlcState State = FVlc::MediaPlayerGetState(Player);

	return (State != ELibvlcState::Opening) && (State != ELibvlcState::Buffering) && (State != ELibvlcState::Error);
}


bool FVlcMediaPlayer::Open(const FString& Url)
{
	if (Url.IsEmpty())
	{
		return false;
	}

	if (!Url.Contains(TEXT("://")))
	{
		TSharedRef<FArchive, ESPMode::ThreadSafe> Archive = MakeShareable(IFileManager::Get().CreateFileReader(
// temp hack until we have proper media sources
#if PLATFORM_WINDOWS
			*Url.Replace(TEXT("/"), TEXT("\\"))
#else
			*Url
#endif
		));

		return Open(Archive, Url);
	}

	Close();

	return (MediaSource.OpenUrl(Url) && InitializePlayer());
}


bool FVlcMediaPlayer::Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl)
{
	if ((Archive->TotalSize() == 0) || OriginalUrl.IsEmpty())
	{
		return false;
	}

	Close();

	return (MediaSource.OpenArchive(Archive, OriginalUrl) && InitializePlayer());
}


bool FVlcMediaPlayer::Seek(const FTimespan& Time)
{
	if (!IsReady())
	{
		return false;
	}

	FVlc::MediaPlayerSetTime(Player, Time.GetTotalMilliseconds());

	return true;
}


bool FVlcMediaPlayer::SetLooping(bool Looping)
{
	ShouldLoop = Looping;
	return true;
}


bool FVlcMediaPlayer::SetRate(float Rate)
{
	if (Player == nullptr)
	{
		return false;
	}

	if ((FVlc::MediaPlayerSetRate(Player, Rate) == -1))
	{
		return false;
	}

	if (FMath::IsNearlyZero(Rate))
	{
		if (IsPlaying())
		{
			if (FVlc::MediaPlayerCanPause(Player) == 0)
			{
				return false;
			}

			FVlc::MediaPlayerPause(Player);
		}
	}
	else if (!IsPlaying())
	{
		if (FVlc::MediaPlayerPlay(Player) == -1)
		{
			return false;
		}
	}

	DesiredRate = Rate;

	return true;
}


/* FVlcMediaPlayer implementation
 *****************************************************************************/

bool FVlcMediaPlayer::InitializePlayer()
{
	Player = FVlc::MediaPlayerNewFromMedia(MediaSource.GetMedia());

	if (Player == nullptr)
	{
		UE_LOG(LogVlcMedia, Warning, TEXT("Failed to initialize media player: %s"), ANSI_TO_TCHAR(FVlc::Errmsg()));
		Close();

		return false;
	}

	// attach callback handlers
	AudioHandler.Initialize(Player);
	VideoHandler.Initialize(Player);

	// attach to event managers
	FLibvlcEventManager* MediaEventManager = FVlc::MediaEventManager(MediaSource.GetMedia());
	FLibvlcEventManager* PlayerEventManager = FVlc::MediaPlayerEventManager(Player);

	if ((MediaEventManager == nullptr) || (PlayerEventManager == nullptr))
	{
		Close();

		return false;
	}

	FVlc::EventAttach(MediaEventManager, ELibvlcEventType::MediaParsedChanged, &FVlcMediaPlayer::HandleEventCallback, this);
	FVlc::EventAttach(PlayerEventManager, ELibvlcEventType::MediaPlayerEndReached, &FVlcMediaPlayer::HandleEventCallback, this);
	FVlc::EventAttach(PlayerEventManager, ELibvlcEventType::MediaPlayerPlaying, &FVlcMediaPlayer::HandleEventCallback, this);
	FVlc::EventAttach(PlayerEventManager, ELibvlcEventType::MediaPlayerPositionChanged, &FVlcMediaPlayer::HandleEventCallback, this);

	MediaEvent.Broadcast(EMediaEvent::MediaOpened);

	return true;
}


/* FVlcMediaPlayer callbacks
 *****************************************************************************/

bool FVlcMediaPlayer::HandleTicker(float DeltaTime)
{
	// process events
	ELibvlcEventType Event;

	while (Events.Dequeue(Event))
	{
		switch (Event)
		{
		case ELibvlcEventType::MediaParsedChanged:
			MediaEvent.Broadcast(EMediaEvent::TracksChanged);
			break;

		case ELibvlcEventType::MediaPlayerEndReached:
			MediaEvent.Broadcast(EMediaEvent::PlaybackEndReached);

			// this causes a short delay, but there seems to be no other way.
			// looping via VLC media list players is also broken. sadness.
			FVlc::MediaPlayerStop(Player);

			if (ShouldLoop && (DesiredRate != 0.0f))
			{
				SetRate(DesiredRate);
			}
			break;

		case ELibvlcEventType::MediaPlayerPlaying:
			LastPlatformSeconds = FPlatformTime::Seconds();
			break;

		case ELibvlcEventType::MediaPlayerPositionChanged:
			CurrentTime = FTimespan::FromMilliseconds(FMath::Max<int64>(0, FVlc::MediaPlayerGetTime(Player)));
			LastPlatformSeconds = FPlatformTime::Seconds();
			break;

		default:
			continue;
		}
	}

	if (!IsPlaying())
	{
		return true;
	}

	// interpolate time (VLC's timer is low-res)
	double PlatformSeconds = FPlatformTime::Seconds();
	CurrentTime += FTimespan::FromSeconds(DesiredRate * (PlatformSeconds - LastPlatformSeconds));
	LastPlatformSeconds = PlatformSeconds;

	// update tracks
	AudioHandler.SetTime(CurrentTime);
	VideoHandler.SetTime(CurrentTime);

	return true;
}


/* FVlcMediaPlayer static functions
 *****************************************************************************/

void FVlcMediaPlayer::HandleEventCallback(FLibvlcEvent* Event, void* UserData)
{
	auto MediaPlayer = (FVlcMediaPlayer*)UserData;

	if (MediaPlayer != nullptr)
	{
		MediaPlayer->Events.Enqueue(Event->Type);

		if (Event->Type == ELibvlcEventType::MediaParsedChanged)
		{
			MediaPlayer->AudioHandler.InitializeTracks();
			MediaPlayer->VideoHandler.InitializeTracks();
		}
	}
}


#undef LOCTEXT_NAMESPACE
