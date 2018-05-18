// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VlcMediaPlayer.h"
#include "VlcMediaPrivate.h"

#include "IMediaEventSink.h"
#include "IMediaOptions.h"
#include "Misc/FileHelper.h"
#include "Serialization/ArrayReader.h"

#include "Vlc.h"
#include "VlcMediaUtils.h"


/* FVlcMediaPlayer structors
 *****************************************************************************/

FVlcMediaPlayer::FVlcMediaPlayer(IMediaEventSink& InEventSink, FLibvlcInstance* InVlcInstance)
	: CurrentRate(0.0f)
	, CurrentTime(FTimespan::Zero())
	, EventSink(InEventSink)
	, MediaSource(InVlcInstance)
	, Player(nullptr)
	, ShouldLoop(false)
{ }


FVlcMediaPlayer::~FVlcMediaPlayer()
{
	Close();
}


/* IMediaControls interface
 *****************************************************************************/

bool FVlcMediaPlayer::CanControl(EMediaControl Control) const
{
	if (Player == nullptr)
	{
		return false;
	}

	if (Control == EMediaControl::Pause)
	{
		return (FVlc::MediaPlayerCanPause(Player) != 0);
	}

	if (Control == EMediaControl::Resume)
	{
		return (FVlc::MediaPlayerGetState(Player) != ELibvlcState::Playing);
	}

	if ((Control == EMediaControl::Scrub) || (Control == EMediaControl::Seek))
	{
		return (FVlc::MediaPlayerIsSeekable(Player) != 0);
	}

	return false;
}


FTimespan FVlcMediaPlayer::GetDuration() const
{
	return MediaSource.GetDuration();
}


float FVlcMediaPlayer::GetRate() const
{
	return CurrentRate;
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


EMediaStatus FVlcMediaPlayer::GetStatus() const
{
	return (GetState() == EMediaState::Preparing) ? EMediaStatus::Buffering : EMediaStatus::None;
}


TRangeSet<float> FVlcMediaPlayer::GetSupportedRates(EMediaRateThinning Thinning) const
{
	TRangeSet<float> Result;

	if (Thinning == EMediaRateThinning::Thinned)
	{
		Result.Add(TRange<float>::Inclusive(0.0f, 10.0f));
	}
	else
	{
		Result.Add(TRange<float>::Inclusive(0.0f, 1.0f));
	}

	return Result;
}


FTimespan FVlcMediaPlayer::GetTime() const
{
	return CurrentTime;
}


bool FVlcMediaPlayer::IsLooping() const
{
	return ShouldLoop;
}


bool FVlcMediaPlayer::Seek(const FTimespan& Time)
{
	ELibvlcState State = FVlc::MediaPlayerGetState(Player);

	if ((State == ELibvlcState::Opening) ||
		(State == ELibvlcState::Buffering) ||
		(State == ELibvlcState::Error))
	{
		return false;
	}

	if (Time != CurrentTime)
	{
		FVlc::MediaPlayerSetTime(Player, Time.GetTotalMilliseconds());
		CurrentTime = Time;
	}

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

	return true;
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
	Callbacks.Shutdown();
	Tracks.Shutdown();
	View.Shutdown();

	// release player
	FVlc::MediaPlayerStop(Player);
	FVlc::MediaPlayerRelease(Player);
	Player = nullptr;

	// reset fields
	CurrentRate = 0.0f;
	CurrentTime = FTimespan::Zero();
	MediaSource.Close();
	Info.Empty();

	// notify listeners
	EventSink.ReceiveMediaEvent(EMediaEvent::TracksChanged);
	EventSink.ReceiveMediaEvent(EMediaEvent::MediaClosed);
}


IMediaCache& FVlcMediaPlayer::GetCache()
{
	return *this;
}


IMediaControls& FVlcMediaPlayer::GetControls() 
{
	return *this;
}


FString FVlcMediaPlayer::GetInfo() const
{
	return Info;
}


FName FVlcMediaPlayer::GetPlayerName() const
{
	static FName PlayerName(TEXT("VlcMedia"));
	return PlayerName;
}


IMediaSamples& FVlcMediaPlayer::GetSamples()
{
	return Callbacks.GetSamples();
}


FString FVlcMediaPlayer::GetStats() const
{
	FLibvlcMedia* Media = MediaSource.GetMedia();

	if (Media == nullptr)
	{
		return TEXT("No media opened.");
	}

	FLibvlcMediaStats Stats;
	
	if (!FVlc::MediaGetStats(Media, &Stats))
	{
		return TEXT("Stats currently not available.");
	}

	FString StatsString;
	{
		StatsString += TEXT("General\n");
		StatsString += FString::Printf(TEXT("    Decoded Video: %i\n"), Stats.DecodedVideo);
		StatsString += FString::Printf(TEXT("    Decoded Audio: %i\n"), Stats.DecodedAudio);
		StatsString += FString::Printf(TEXT("    Displayed Pictures: %i\n"), Stats.DisplayedPictures);
		StatsString += FString::Printf(TEXT("    Lost Pictures: %i\n"), Stats.LostPictures);
		StatsString += FString::Printf(TEXT("    Played A-Buffers: %i\n"), Stats.PlayedAbuffers);
		StatsString += FString::Printf(TEXT("    Lost Lost A-Buffers: %i\n"), Stats.LostAbuffers);
		StatsString += TEXT("\n");

		StatsString += TEXT("Input\n");
		StatsString += FString::Printf(TEXT("    Bit Rate: %i\n"), Stats.InputBitrate);
		StatsString += FString::Printf(TEXT("    Bytes Read: %i\n"), Stats.ReadBytes);
		StatsString += TEXT("\n");

		StatsString += TEXT("Demux\n");
		StatsString += FString::Printf(TEXT("    Bit Rate: %f\n"), Stats.DemuxBitrate);
		StatsString += FString::Printf(TEXT("    Bytes Read: %i\n"), Stats.DemuxReadBytes);
		StatsString += FString::Printf(TEXT("    Corrupted: %i\n"), Stats.DemuxCorrupted);
		StatsString += FString::Printf(TEXT("    Discontinuity: %i\n"), Stats.DemuxDiscontinuity);
		StatsString += TEXT("\n");

		StatsString += TEXT("Network\n");
		StatsString += FString::Printf(TEXT("    Bitrate: %f\n"), Stats.SendBitrate);
		StatsString += FString::Printf(TEXT("    Sent Bytes: %i\n"), Stats.SentBytes);
		StatsString += FString::Printf(TEXT("    Sent Packets: %i\n"), Stats.SentPackets);
		StatsString += TEXT("\n");
	}

	return StatsString;
}


IMediaTracks& FVlcMediaPlayer::GetTracks()
{
	return Tracks;
}


FString FVlcMediaPlayer::GetUrl() const
{
	return MediaSource.GetCurrentUrl();
}


IMediaView& FVlcMediaPlayer::GetView()
{
	return View;
}


bool FVlcMediaPlayer::Open(const FString& Url, const IMediaOptions* Options)
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

		if ((Options != nullptr) && Options->GetMediaOption("PrecacheFile", false))
		{
			FArrayReader* Reader = new FArrayReader;

			if (FFileHelper::LoadFileToArray(*Reader, FilePath))
			{
				Archive = MakeShareable(Reader);
			}
			else
			{
				delete Reader;
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
	else if (!MediaSource.OpenUrl(Url))
	{
		return false;
	}

	return InitializePlayer();
}


bool FVlcMediaPlayer::Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl, const IMediaOptions* /*Options*/)
{
	Close();

	if (OriginalUrl.IsEmpty() || !MediaSource.OpenArchive(Archive, OriginalUrl))
	{
		return false;
	}
	
	return InitializePlayer();
}


void FVlcMediaPlayer::TickInput(FTimespan DeltaTime, FTimespan /*Timecode*/)
{
	if (Player == nullptr)
	{
		return;
	}

	// process events
	ELibvlcEventType Event;

	while (Events.Dequeue(Event))
	{
		switch (Event)
		{
		case ELibvlcEventType::MediaParsedChanged:
			Tracks.Initialize(*Player, Info);
			Callbacks.Initialize(*Player);
			View.Initialize(*Player);
			EventSink.ReceiveMediaEvent(EMediaEvent::TracksChanged);
			break;

		case ELibvlcEventType::MediaPlayerEndReached:
			// begin hack: this causes a short delay, but there seems to be no
			// other way. looping via VLC Media List players is also broken :(
			FVlc::MediaPlayerStop(Player);
			// end hack

			Callbacks.GetSamples().FlushSamples();
			EventSink.ReceiveMediaEvent(EMediaEvent::PlaybackEndReached);

			if (ShouldLoop && (CurrentRate != 0.0f))
			{
				CurrentTime = FTimespan::Zero();
				SetRate(CurrentRate);
			}
			else
			{
				EventSink.ReceiveMediaEvent(EMediaEvent::PlaybackSuspended);
			}
			break;

		case ELibvlcEventType::MediaPlayerPaused:
			EventSink.ReceiveMediaEvent(EMediaEvent::PlaybackSuspended);
			break;

		case ELibvlcEventType::MediaPlayerPlaying:
			EventSink.ReceiveMediaEvent(EMediaEvent::PlaybackResumed);
			break;

		default:
			continue;
		}
	}

	const ELibvlcState State = FVlc::MediaPlayerGetState(Player);

	// update current time & rate
	if (State == ELibvlcState::Playing)
	{
		CurrentRate = FVlc::MediaPlayerGetRate(Player);
		CurrentTime += DeltaTime * CurrentRate;
	}
	else
	{
		CurrentRate = 0.0f;
	}

	Callbacks.SetCurrentTime(CurrentTime);
}


/* FVlcMediaPlayer implementation
 *****************************************************************************/

bool FVlcMediaPlayer::InitializePlayer()
{
	// create player for media source
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
	FVlc::EventAttach(PlayerEventManager, ELibvlcEventType::MediaPlayerStopped, &FVlcMediaPlayer::StaticEventCallback, this);

	// initialize player
	CurrentRate = 0.0f;
	CurrentTime = FTimespan::Zero();

	EventSink.ReceiveMediaEvent(EMediaEvent::MediaOpened);

	return true;
}


/* FVlcMediaPlayer static functions
 *****************************************************************************/

void FVlcMediaPlayer::StaticEventCallback(FLibvlcEvent* Event, void* UserData)
{
	if (Event == nullptr)
	{
		return;
	}

	UE_LOG(LogVlcMedia, Verbose, TEXT("Player %llx: Event [%s]"), UserData, *VlcMedia::EventToString(Event));

	if (UserData != nullptr)
	{
		((FVlcMediaPlayer*)UserData)->Events.Enqueue(Event->Type);
	}
}
