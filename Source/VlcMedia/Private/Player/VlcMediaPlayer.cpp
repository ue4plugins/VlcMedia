// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPCH.h"
#include "Vlc.h"
#include "VlcMediaPlayer.h"
#include "VlcMediaUtils.h"


/* FVlcMediaPlayer structors
 *****************************************************************************/


FVlcMediaPlayer::FVlcMediaPlayer(FLibvlcInstance* InVlcInstance)
	: CurrentTime(FTimespan::Zero())
	, DesiredRate(0.0)
	, LastPlatformSeconds(0.0)
	, MediaSource(InVlcInstance)
	, Player(nullptr)
	, ShouldLoop(false)
{ }


FVlcMediaPlayer::~FVlcMediaPlayer()
{
	Close();
}


/* FTickerObjectBase interface
 *****************************************************************************/

bool FVlcMediaPlayer::Tick(float DeltaTime)
{
	if (Player == nullptr)
	{
		return true;
	}

	// interpolate time, because FVlc::MediaPlayerGetTime is too low-res
	if (FVlc::MediaPlayerGetState(Player) == ELibvlcState::Playing)
	{
		double PlatformSeconds = FPlatformTime::Seconds();
		CurrentTime += FTimespan::FromSeconds(DesiredRate * (PlatformSeconds - LastPlatformSeconds));
		LastPlatformSeconds = PlatformSeconds;
	}

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
			// begin hack: this causes a short delay, but there seems to be no
			// other way. looping via VLC Media List players is also broken :(
			FVlc::MediaPlayerStop(Player);
			// end hack

			if (ShouldLoop && (DesiredRate != 0.0f))
			{
				SetRate(DesiredRate);
			}

			MediaEvent.Broadcast(EMediaEvent::PlaybackSuspended);
			MediaEvent.Broadcast(EMediaEvent::PlaybackEndReached);
			break;

		case ELibvlcEventType::MediaPlayerPaused:
			LastPlatformSeconds = FPlatformTime::Seconds();
			MediaEvent.Broadcast(EMediaEvent::PlaybackSuspended);
			break;

		case ELibvlcEventType::MediaPlayerPlaying:
			LastPlatformSeconds = FPlatformTime::Seconds();
			MediaEvent.Broadcast(EMediaEvent::PlaybackResumed);
			break;

		case ELibvlcEventType::MediaPlayerPositionChanged:
			CurrentTime = FTimespan::FromMilliseconds(FMath::Max<int64>(0, FVlc::MediaPlayerGetTime(Player)));
			LastPlatformSeconds = FPlatformTime::Seconds();
			break;

		default:
			continue;
		}
	}

	return true;
}


/* IMediaControls interface
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


float FVlcMediaPlayer::GetRate() const
{
	if ((Player == nullptr) || (FVlc::MediaPlayerGetState(Player) != ELibvlcState::Playing))
	{
		return 0.0f;
	}

	return FVlc::MediaPlayerGetRate(Player);
}


EMediaState FVlcMediaPlayer::GetState() const
{
	if (Player == nullptr)
	{
		return EMediaState::Closed;
	}

	ELibvlcState State = FVlc::MediaPlayerGetState(Player);

	switch (State)
	{
	case ELibvlcState::Error:
		return EMediaState::Error;

	case ELibvlcState::Buffering:
	case ELibvlcState::Opening:
		return EMediaState::Preparing;

	case ELibvlcState::Paused:
		return EMediaState::Paused;

	case ELibvlcState::Playing:
		return EMediaState::Playing;

	case ELibvlcState::Ended:
	case ELibvlcState::NothingSpecial:
	case ELibvlcState::Stopped:
		return EMediaState::Stopped;
	}

	return EMediaState::Error; // should never get here
}


TRange<float> FVlcMediaPlayer::GetSupportedRates(EMediaPlaybackDirections Direction, bool Unthinned) const
{
	if (Direction == EMediaPlaybackDirections::Reverse)
	{
		return TRange<float>::Empty();
	}

	return TRange<float>(0.0f, 10.0f);
}


FTimespan FVlcMediaPlayer::GetTime() const
{
	return CurrentTime;
}


bool FVlcMediaPlayer::IsLooping() const
{
	return ShouldLoop;
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
		if (FVlc::MediaPlayerGetState(Player) == ELibvlcState::Playing)
		{
			if (FVlc::MediaPlayerCanPause(Player) == 0)
			{
				return false;
			}

			FVlc::MediaPlayerPause(Player);
		}
	}
	else if (FVlc::MediaPlayerGetState(Player) != ELibvlcState::Playing)
	{
		if (FVlc::MediaPlayerPlay(Player) == -1)
		{
			return false;
		}
	}

	DesiredRate = Rate;

	return true;
}


bool FVlcMediaPlayer::SupportsRate(float Rate, bool Unthinned) const
{
	return (Rate >= 0.0f) && (Rate <= 10.0f);
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
	Output.Shutdown();
	Tracks.Shutdown();

	// release player
	FVlc::MediaPlayerStop(Player);
	FVlc::MediaPlayerRelease(Player);
	Player = nullptr;

	// reset fields
	CurrentTime = FTimespan::Zero();
	MediaSource.Close();

	// notify listeners
	MediaEvent.Broadcast(EMediaEvent::TracksChanged);
	MediaEvent.Broadcast(EMediaEvent::MediaClosed);
}


IMediaControls& FVlcMediaPlayer::GetControls() 
{
	return *this;
}


FString FVlcMediaPlayer::GetInfo() const
{
	return TEXT("VlcMedia media information not implemented yet");
}


IMediaOutput& FVlcMediaPlayer::GetOutput()
{
	return Output;
}


FString FVlcMediaPlayer::GetStats() const
{
	return TEXT("VlcMedia stats information not implemented yet");
}


IMediaTracks& FVlcMediaPlayer::GetTracks()
{
	return Tracks;
}


FString FVlcMediaPlayer::GetUrl() const
{
	return MediaSource.GetCurrentUrl();
}


bool FVlcMediaPlayer::Open(const FString& Url, const IMediaOptions& Options)
{
	Close();

	if (Url.IsEmpty())
	{
		return false;
	}

	if (Url.StartsWith(TEXT("file://")))
	{
		// open local files via platform file system
		TSharedPtr<FArchive, ESPMode::ThreadSafe> Archive;
		const TCHAR* FilePath = &Url[7];

		if (Options.GetMediaOption("PrecacheFile", false))
		{
			FBufferArchive* Buffer = new FBufferArchive;

			if (FFileHelper::LoadFileToArray(*Buffer, FilePath))
			{
				Archive = MakeShareable(Buffer);
			}
			else
			{
				delete Buffer;
			}
		}
		else
		{
			Archive = MakeShareable(IFileManager::Get().CreateFileReader(FilePath));
		}

		if (!Archive.IsValid())
		{
			UE_LOG(LogVlcMedia, Warning, TEXT("Failed to open media file: %s"), FilePath);

			return false;
		}

		if (!MediaSource.OpenArchive(Archive.ToSharedRef(), Url))
		{
			return false;
		}
	}
	else
	{
		if (!MediaSource.OpenUrl(Url))
		{
			return false;
		}
	}
	
	return InitializePlayer();
}


bool FVlcMediaPlayer::Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl, const IMediaOptions& Options)
{
	Close();

	if (OriginalUrl.IsEmpty() || !MediaSource.OpenArchive(Archive, OriginalUrl))
	{
		return false;
	}
	
	return InitializePlayer();
}


bool FVlcMediaPlayer::Seek(const FTimespan& Time)
{
	ELibvlcState State = FVlc::MediaPlayerGetState(Player);

	if ((State != ELibvlcState::Opening) ||
		(State == ELibvlcState::Buffering) ||
		(State == ELibvlcState::Error))
	{
		return false;
	}

	FVlc::MediaPlayerSetTime(Player, Time.GetTotalMilliseconds());

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

		return false;
	}

	// attach to event managers
	FLibvlcEventManager* MediaEventManager = FVlc::MediaEventManager(MediaSource.GetMedia());
	FLibvlcEventManager* PlayerEventManager = FVlc::MediaPlayerEventManager(Player);

	if ((MediaEventManager == nullptr) || (PlayerEventManager == nullptr))
	{
		FVlc::MediaPlayerRelease(Player);
		Player = nullptr;

		return false;
	}

	FVlc::EventAttach(MediaEventManager, ELibvlcEventType::MediaParsedChanged, &FVlcMediaPlayer::StaticEventCallback, this);
	FVlc::EventAttach(PlayerEventManager, ELibvlcEventType::MediaPlayerEndReached, &FVlcMediaPlayer::StaticEventCallback, this);
	FVlc::EventAttach(PlayerEventManager, ELibvlcEventType::MediaPlayerPlaying, &FVlcMediaPlayer::StaticEventCallback, this);
	FVlc::EventAttach(PlayerEventManager, ELibvlcEventType::MediaPlayerPositionChanged, &FVlcMediaPlayer::StaticEventCallback, this);

	MediaEvent.Broadcast(EMediaEvent::MediaOpened);

	return true;
}


/* FVlcMediaPlayer static functions
 *****************************************************************************/

void FVlcMediaPlayer::StaticEventCallback(FLibvlcEvent* Event, void* UserData)
{
	UE_LOG(LogVlcMedia, Verbose, TEXT("LibVLC event: %s"), *VlcMedia::EventToString(Event));

	auto MediaPlayer = (FVlcMediaPlayer*)UserData;

	if (MediaPlayer == nullptr)
	{
		return;
	}

	if (Event->Type == ELibvlcEventType::MediaParsedChanged)
	{
		MediaPlayer->Tracks.Initialize(*MediaPlayer->Player);
		MediaPlayer->Output.Initialize(*MediaPlayer->Player);
	}
	else if (Event->Type == ELibvlcEventType::MediaPlayerPlaying)
	{
		MediaPlayer->Output.Resume(MediaPlayer->CurrentTime);
	}

	MediaPlayer->Events.Enqueue(Event->Type);
}
