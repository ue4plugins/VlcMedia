// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VlcMediaPlayer.h"
#include "VlcMediaPrivate.h"

#include "IMediaEventSink.h"
#include "IMediaOptions.h"
#include "Misc/Guid.h"
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
	, bVideoFormatSet(false)
	, bCallbacksInitialized(false)
	, LastCallbackResetSeconds(0.0)
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

	// [DEBUG] SetRate 호출 로깅
	UE_LOG(LogVlcMedia, Warning, TEXT("[VlcMediaPlayer] SetRate called with Rate=%f"), Rate);

	if ((FVlc::MediaPlayerSetRate(Player, Rate) == -1))
	{
		UE_LOG(LogVlcMedia, Warning, TEXT("[VlcMediaPlayer] MediaPlayerSetRate failed"));
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
			UE_LOG(LogVlcMedia, Warning, TEXT("[VlcMediaPlayer] MediaPlayerPlay failed"));
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

FGuid FVlcMediaPlayer::GetPlayerPluginGUID() const
{
	static const FGuid PluginGuid(0x6BD4A7B8, 0x2C984F7F, 0xA31B6F8E, 0xB2F5B6A1);
	return PluginGuid;
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
		StatsString += FString::Printf(TEXT("    Decoded Video: %d\n"), Stats.DecodedVideo);
		StatsString += FString::Printf(TEXT("    Decoded Audio: %d\n"), Stats.DecodedAudio);
		StatsString += FString::Printf(TEXT("    Displayed Pictures: %d\n"), Stats.DisplayedPictures);
		StatsString += FString::Printf(TEXT("    Lost Pictures: %d\n"), Stats.LostPictures);
		StatsString += FString::Printf(TEXT("    Played A-Buffers: %d\n"), Stats.PlayedAbuffers);
		StatsString += FString::Printf(TEXT("    Lost Lost A-Buffers: %d\n"), Stats.LostAbuffers);
		StatsString += TEXT("\n");

		StatsString += TEXT("Input\n");
		StatsString += FString::Printf(TEXT("    Bit Rate: %f\n"), Stats.InputBitrate);
		StatsString += FString::Printf(TEXT("    Bytes Read: %d\n"), Stats.ReadBytes);
		StatsString += TEXT("\n");

		StatsString += TEXT("Demux\n");
		StatsString += FString::Printf(TEXT("    Bit Rate: %f\n"), Stats.DemuxBitrate);
		StatsString += FString::Printf(TEXT("    Bytes Read: %d\n"), Stats.DemuxReadBytes);
		StatsString += FString::Printf(TEXT("    Corrupted: %d\n"), Stats.DemuxCorrupted);
		StatsString += FString::Printf(TEXT("    Discontinuity: %d\n"), Stats.DemuxDiscontinuity);
		StatsString += TEXT("\n");

		StatsString += TEXT("Network\n");
		StatsString += FString::Printf(TEXT("    Bitrate: %f\n"), Stats.SendBitrate);
		StatsString += FString::Printf(TEXT("    Sent Bytes: %d\n"), Stats.SentBytes);
		StatsString += FString::Printf(TEXT("    Sent Packets: %d\n"), Stats.SentPackets);
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

    // [DEBUG] TickInput 호출 확인 (매 1초마다)
    static double LastTickLogTime = 0.0;
    double CurrentPlatformTime = FPlatformTime::Seconds();
    if (CurrentPlatformTime - LastTickLogTime >= 1.0)
    {
        ELibvlcState DebugState = FVlc::MediaPlayerGetState(Player);
        UE_LOG(LogVlcMedia, Warning, TEXT("[VLC TickInput Entry] Player=%p, LibVlcState=%d"), this, (int32)DebugState);
        LastTickLogTime = CurrentPlatformTime;
    }

	// process events
	ELibvlcEventType Event;

	while (Events.Dequeue(Event))
	{
		switch (Event)
		{
		case ELibvlcEventType::MediaParsedChanged:
			UE_LOG(LogVlcMedia, Warning, TEXT("Player %p: MediaParsedChanged - Initializing Tracks, Callbacks, View"), this);
			Tracks.Initialize(*Player, Info);
			Callbacks.Initialize(*Player);
			View.Initialize(*Player);
			bVideoFormatSet = false;
			bCallbacksInitialized = true;
			LastCallbackResetSeconds = FPlatformTime::Seconds();

			{
				uint32 Width = 0;
				uint32 Height = 0;
				if (FVlc::VideoGetSize(Player, 0, &Width, &Height) != 0 || Width == 0 || Height == 0)
				{
					Width = 640;
					Height = 480;
				}

				FVlc::VideoSetFormat(Player, "RV32", Width, Height, Width * 4);
				bVideoFormatSet = true;
				UE_LOG(LogVlcMedia, Warning, TEXT("Set VLC video format to RV32 (%ux%u) after MediaParsedChanged"), Width, Height);
			}

			if (Tracks.GetNumTracks(EMediaTrackType::Video) > 0)
			{
				if (Tracks.SelectTrack(EMediaTrackType::Video, 0))
				{
					UE_LOG(LogVlcMedia, Warning, TEXT("Selected default video track 0"));
				}
				else
				{
					UE_LOG(LogVlcMedia, Warning, TEXT("Failed to select default video track 0"));
				}
			}
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
            UE_LOG(LogVlcMedia, Verbose, TEXT("Player %p: Playing (%s)"), this, *MediaSource.GetCurrentUrl());
			EventSink.ReceiveMediaEvent(EMediaEvent::PlaybackResumed);
			break;
            
		case ELibvlcEventType::MediaPlayerStopped:
			UE_LOG(LogVlcMedia, Verbose, TEXT("Player %p: Stopped"), this);
			EventSink.ReceiveMediaEvent(EMediaEvent::PlaybackSuspended);
			break;

		default:
			continue;
		}
	}

	ELibvlcState State = FVlc::MediaPlayerGetState(Player);

	if ((State == ELibvlcState::Playing) && (Callbacks.GetSamples().NumVideoSamples() == 0))
	{
		const double Now = FPlatformTime::Seconds();
		if (Now - LastCallbackResetSeconds > 2.0)
		{
			UE_LOG(LogVlcMedia, Warning, TEXT("No video samples yet; reinitializing callbacks and forcing RV32 format"));
			Callbacks.Initialize(*Player);
			bCallbacksInitialized = true;
			bVideoFormatSet = false;
			LastCallbackResetSeconds = Now;
		}
	}

	// ensure VLC uses a CPU-friendly format once dimensions are known
	if (!bVideoFormatSet)
	{
		uint32 Width = 0;
		uint32 Height = 0;
		if (FVlc::VideoGetSize(Player, 0, &Width, &Height) == 0 && Width > 0 && Height > 0)
		{
			FVlc::VideoSetFormat(Player, "RV32", Width, Height, Width * 4);
			bVideoFormatSet = true;
			UE_LOG(LogVlcMedia, Warning, TEXT("Set VLC video format to RV32 (%ux%u)"), Width, Height);
		}
	}

    // [DEBUG] State 체크 주석 처리하여 강제 진행 (디버깅용)
	// if ((State != ELibvlcState::Opening) && (State != ELibvlcState::Buffering) && (State != ELibvlcState::Playing))
	// {
	// 	return;
	// }

	if (ShouldLoop && (State == ELibvlcState::Ended))
	{
		FVlc::MediaPlayerSetPosition(Player, 0.0f);
	}

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
    
    // [DEBUG] 샘플 큐 상태 로깅 (1초마다)
	static FTimespan LastEndLogTime = FTimespan::Zero();
	if ((CurrentTime - LastEndLogTime).GetTotalSeconds() >= 1.0)
	{
		int32 VideoSamples = Callbacks.GetSamples().NumVideoSamples();
		int32 AudioSamples = Callbacks.GetSamples().NumAudioSamples();
        int32 SelectedVideoTrack = Tracks.GetSelectedTrack(EMediaTrackType::Video);
		
		UE_LOG(LogVlcMedia, Warning, TEXT("[VLC TickInput End] Player=%p, State=%d, VideoSamples=%d, AudioSamples=%d, SelectedTrack=%d, CurrentTime=%s"),
			this, (int32)State, VideoSamples, AudioSamples, SelectedVideoTrack, *CurrentTime.ToString());
		
		LastEndLogTime = CurrentTime;
	}
}

bool FVlcMediaPlayer::GetPlayerFeatureFlag(EFeatureFlag Flag) const
{
	switch (Flag)
	{
	case EFeatureFlag::AlwaysPullNewestVideoFrame:
	case EFeatureFlag::UseRealtimeWithVideoOnly:
	case EFeatureFlag::PlayerSelectsDefaultTracks:
		return true;
	default:
		return false;
	}
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

	// register callbacks early so VLC uses vmem for video output
	Callbacks.Initialize(*Player);
	bCallbacksInitialized = true;
	LastCallbackResetSeconds = FPlatformTime::Seconds();

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

	UE_LOG(LogVlcMedia, Verbose, TEXT("Player %p: Event [%s]"), UserData, *VlcMedia::EventToString(Event));

	if (UserData != nullptr)
	{
		((FVlcMediaPlayer*)UserData)->Events.Enqueue(Event->Type);
	}
}
