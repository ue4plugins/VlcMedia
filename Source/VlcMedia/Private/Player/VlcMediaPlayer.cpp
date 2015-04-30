// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.
// modified by @anselm

#include "VlcMediaPrivatePCH.h"
#include <mutex>

#define LOCTEXT_NAMESPACE "FVlcMediaModule"

// xxx TODO unsure exactly the right thing to do there
#define VIDEOWIDTH 1280 
#define VIDEOHEIGHT 720 

///////////////////////////////////////////////////////////////////////////////////////////

class FVlcMediaPlayer::MediaTrack
        : public IMediaTrack
{
public:

        MediaTrack(FVlcMediaPlayer & Player, int Index)
                : MediaPlayer(Player)
                , TrackIndex(Index)
                , TrackIsEnabled(true)
        {
		printf("VLC:Media track started\n");
        }

        virtual void AddSink(const IMediaSinkRef& Sink)
        {
                Sinks.AddUnique(IMediaSinkWeakPtr(Sink));
        }

        virtual bool Disable()
        {
                TrackIsEnabled = false;
                return MediaPlayer.MediaState != EMediaState::Error;
        }

        virtual bool Enable()
        {
                TrackIsEnabled = true;
                return MediaPlayer.MediaState != EMediaState::Error;
        }

        virtual const IMediaTrackAudioDetails& GetAudioDetails() const
        {
                verify(false); // not an audio track
                return *static_cast<IMediaTrackAudioDetails*>(nullptr);
        }

        virtual const IMediaTrackCaptionDetails& GetCaptionDetails() const
        {
                verify(false); // not a caption track
                return *static_cast<IMediaTrackCaptionDetails*>(nullptr);
        }

        virtual FText GetDisplayName() const
        {
                return FText::Format(LOCTEXT("UnnamedTrackFormat", "Unnamed Track {0}"), FText::AsNumber(GetIndex()));
        }

        virtual uint32 GetIndex() const
        {
                return TrackIndex;
        }

        virtual FString GetLanguage() const
        {
                return TEXT("und");
        }

        virtual FString GetName() const
        {
                return FPaths::GetBaseFilename(MediaPlayer.MediaUrl);
        }

        virtual const IMediaTrackVideoDetails& GetVideoDetails() const
        {
                verify(false); // not a video track
                return *static_cast<IMediaTrackVideoDetails*>(nullptr);
        }

        virtual bool IsEnabled() const
        {
                return TrackIsEnabled;
        }

        virtual bool IsMutuallyExclusive(const IMediaTrackRef& Other) const
        {
                return false;
        }

        virtual bool IsProtected() const
        {
                return false;
        }

        virtual void RemoveSink(const IMediaSinkRef& Sink)
        {
                Sinks.RemoveSingle(IMediaSinkWeakPtr(Sink));
        }

        virtual void Tick(float DeltaTime)
        {
        }

        virtual bool TickInRenderThread()
        {
                return false;
        }


        virtual void ProcessMediaSample(void* SampleBuffer, uint32 SampleSize, FTimespan SampleDuration, FTimespan SampleTime)
        {
                for (IMediaSinkWeakPtr& SinkPtr : Sinks)
                {
                        IMediaSinkPtr Sink = SinkPtr.Pin();

                        if (Sink.IsValid())
                        {
                                Sink->ProcessMediaSample(SampleBuffer, SampleSize, FTimespan(SampleDuration), FTimespan(SampleTime));
                        }
                }
        }

protected:

        FVlcMediaPlayer & MediaPlayer;
        int TrackIndex;
        bool TrackIsEnabled;

        // The collection of registered media sinks.
        TArray<IMediaSinkWeakPtr> Sinks;
};


///////////////////////////////////////////////////////////////////////////////////////////


class FVlcMediaPlayer::VideoTrack
	: public FVlcMediaPlayer::MediaTrack
	, IMediaTrackVideoDetails
{

public:

	VideoTrack(FVlcMediaPlayer & Player, int Index):
		MediaTrack(Player,Index),
		LastFramePosition(-1)
	{
		CurrentFramePosition = 0; // hack
		printf("VLC:Media video started\n");
	}

	virtual bool Disable() {
		return true;
	}

	virtual bool Enable() {
		return true;
	}

	virtual EMediaTrackTypes GetType() const
	{
		return EMediaTrackTypes::Video;
	}

	virtual const IMediaTrackVideoDetails& GetVideoDetails() const
	{
		return *this;
	}

	virtual uint32 GetBitRate() const
	{
		return 0;
	}

	virtual FIntPoint GetDimensions() const
	{
		return FIntPoint(VIDEOWIDTH,VIDEOHEIGHT);
	}

	virtual float GetFrameRate() const
	{
		return 30.0f;
	}

	virtual void Tick(float DeltaTime) {
		if(MediaPlayer.MediaState != EMediaState::Error)
		{
			CurrentFramePosition += DeltaTime * 1000;
			if(LastFramePosition != CurrentFramePosition) {
				LastFramePosition = CurrentFramePosition;
				void* SampleData = 0;
				int64 SampleCount = 0;
				if(MediaPlayer.LockGetVideoLastFrameData(SampleData,SampleCount)) {
					ProcessMediaSample(
						SampleData, SampleCount,
						FTimespan::MaxValue(),
						FTimespan::FromMilliseconds(LastFramePosition));
					MediaPlayer.Unlock();
				}
			}
		}
	}

	virtual bool TickInRenderThread()
	{
		return true;
	}

private:

	int32 LastFramePosition;
	int32 CurrentFramePosition;
};

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


class FVlcMediaPlayer::AudioTrack
        : public FVlcMediaPlayer::MediaTrack
        , public IMediaTrackAudioDetails
{
public:

        AudioTrack(FVlcMediaPlayer & Player, int Index)
                : MediaTrack(Player, Index)
        {
        }

        virtual bool Disable()
        {
                if (MediaTrack::Disable())
                {
                        //MediaPlayer.JavaMediaPlayer->SetAudioEnabled(false);
                        return true;
                }
                return false;
        }

        virtual bool Enable()
        {
                if (MediaTrack::Enable())
                {
                        //MediaPlayer.JavaMediaPlayer->SetAudioEnabled(true);
                        return true;
                }
                return false;
        }

        virtual const IMediaTrackAudioDetails& GetAudioDetails() const
        {
                return *this;
        }

        virtual EMediaTrackTypes GetType() const
        {
                return EMediaTrackTypes::Audio;
        }

        virtual uint32 GetNumChannels() const
        {
                return 1;
        }

        virtual uint32 GetSamplesPerSecond() const
        {
                return 0;
        }
};

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

#include <vlc/vlc.h>

#ifdef VLCSDL

#include <SDL/SDL.h>
#include <SDL/SDL_mutex.h>

struct VLCContext {
  SDL_Surface* surf;
  SDL_mutex* mutex;
};

static SDL_Surface* screen;
static SDL_Event event;

static void *vlc_lock(void *data, void **p_pixels)
{
    struct VLCContext *ctx = (struct VLCContext*) data;
    SDL_LockMutex(ctx->mutex);
    SDL_LockSurface(ctx->surf);
    *p_pixels = ctx->surf->pixels;
    return NULL;
}

static void vlc_unlock(void *data, void *id, void *const *p_pixels)
{
    struct VLCContext *ctx = (struct VLCContext *)data;

    uint16_t *pixels = (uint16_t*)*p_pixels;
    int x, y;

    for(y = 10; y < 40; y++)
        for(x = 10; x < 40; x++)
            if(x < 13 || y < 13 || x > 36 || y > 36)
                pixels[y * VIDEOWIDTH + x] = 0xffff;
            else
                pixels[y * VIDEOWIDTH + x] = 0x0;

    SDL_UnlockSurface(ctx->surf);
    SDL_UnlockMutex(ctx->mutex);
}

static void vlc_display(void *data, void *id) {
    (void) data;
}

#else

struct VLCContext {
	void *pixeldata;
	std::mutex mutex;
};

static void* vlc_lock(void* data, void**p_pixels) {
	struct VLCContext *ctx = (struct VLCContext*)data;
	ctx->mutex.lock();
	*p_pixels = ctx->pixeldata;
	return NULL;
}

static void vlc_unlock(void* data, void* id, void *const *p_pixels) {
	struct VLCContext *ctx = (struct VLCContext*)data;
	ctx->mutex.unlock();
}

static void vlc_display(void* data, void* id) {
	(void) data;
}

#endif

bool FVlcMediaPlayer::LockGetVideoLastFrameData(void* &SampleData, int64 &SampleCount) {
	if(!vlcmedia)return false;
#ifdef VLCSDL
	SDL_LockMutex(vlccontext->mutex);
	// SDL_Rect rect = blah
	// SDL_BlitSurface(vlccontext->surf,NULL,screen,&rect);
	SDL_UnlockMutex(vlccontext->mutex);
	// SDL_Flip(screen);
	// SDL_Delay(10);
#else
	vlccontext->mutex.lock();
	SampleData = vlccontext->pixeldata;
	SampleCount = VIDEOWIDTH*VIDEOHEIGHT*4;
	return true;
#endif
}

bool FVlcMediaPlayer::Unlock() {
#ifndef VLCSDL
	vlccontext->mutex.unlock();
#endif
	return true;
}


/* FVlcMediaPlayer structors
 *****************************************************************************/


FVlcMediaPlayer::FVlcMediaPlayer()
	: MediaState(EMediaState::Error)
{
	MediaUrl = FString();
	MediaState = EMediaState::Idle;

	vlccontext = new VLCContext();
	vlchandle = 0;
	vlcmedia = 0;

#ifdef VLCSDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD) == -1) {
      		printf("vlc: cannot initialize SDL\n");
		return;
	}

	vlccontext->surf = SDL_CreateRGBSurface(SDL_SWSURFACE, VIDEOWIDTH, VIDEOHEIGHT, 16, 0x001f, 0x07e0, 0xf800, 0);
	vlccontext->mutex = SDL_CreateMutex();

	// int options = SDL_ANYFORMAT | SDL_HWSURFACE | SDL_DOUBLEBUF;
	// screen = SDL_SetVideoMode(WIDTH, HEIGHT, 0, options);
	// if(!screen) {
	//		printf("cannot set video mode\n");
	// return EXIT_FAILURE;
	// }

#else

	vlccontext->pixeldata = malloc(VIDEOWIDTH*VIDEOHEIGHT*4 * 2); // xxx TODO - verify that it is RGBA

#endif

	char const *vlc_argv[] = {
//		"--verbose=2",
		"--no-video-title-show",
		"--no-stats",
		"--no-sub-autodetect-file",
		"--no-inhibit",
		"--no-disable-screensaver",
		"--no-snapshot-preview",
//		"--intf", "dummy",
	//	"--vout", "dummy",
//		"--no-audio", /* skip any audio track */
		"--no-xlib", /* tell VLC to not use Xlib */
	};
	int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);
	vlchandle = libvlc_new(vlc_argc, vlc_argv);
}

FVlcMediaPlayer::~FVlcMediaPlayer()
{
	Close();
	MediaState = EMediaState::End;
	libvlc_release((libvlc_instance_t*)vlchandle);
	vlchandle = 0;
	free(vlccontext->pixeldata);
	vlccontext->pixeldata = 0;
	free(vlccontext);
	vlccontext =0;
}


/* IMediaInfo interface
 *****************************************************************************/

FTimespan FVlcMediaPlayer::GetDuration() const
{
        libvlc_time_t duration = libvlc_media_player_get_length((libvlc_media_player_t*)vlcmedia);
	return FTimespan::FromMilliseconds(duration);
}


TRange<float> FVlcMediaPlayer::GetSupportedRates( EMediaPlaybackDirections Direction, bool Unthinned ) const
{
	return TRange<float>(1.0f);
}


FString FVlcMediaPlayer::GetUrl() const
{
	return MediaUrl;
}


bool FVlcMediaPlayer::SupportsRate( float Rate, bool Unthinned ) const
{
	return Rate == 1.0f;
}


bool FVlcMediaPlayer::SupportsScrubbing() const
{
	return true;
}


bool FVlcMediaPlayer::SupportsSeeking() const
{
	return true;
}


/* IMediaPlayer interface
 *****************************************************************************/

void FVlcMediaPlayer::Close()
{
	if(!vlcmedia) return;
	MediaUrl = FString();
	MediaState = EMediaState::Idle;
	ClosedEvent.Broadcast();
        for (IMediaTrackRef& track : Tracks) {
            MediaTrack &t = static_cast<MediaTrack&>(*track);
	    // XXX TODO free these
	}
	Tracks.Reset();
	libvlc_media_player_stop((libvlc_media_player_t*)vlcmedia);
	libvlc_video_set_callbacks((libvlc_media_player_t*)vlcmedia, 0, 0, 0, 0);
	libvlc_media_player_release((libvlc_media_player_t*)vlcmedia);
	vlcmedia = 0;
#ifdef VLCSDL
	SDL_DestroyMutex(vlccontext->mutex);
	SDL_FreeSurface(vlccontext->surf);
	SDL_Quit();
#endif
}

const IMediaInfo& FVlcMediaPlayer::GetMediaInfo() const 
{
	return *this;
}


float FVlcMediaPlayer::GetRate() const
{
	if(!vlcmedia)return 0;
	return libvlc_media_player_get_rate((libvlc_media_player_t*)vlcmedia);
}


FTimespan FVlcMediaPlayer::GetTime() const 
{
        if (MediaState != EMediaState::Error)
        {
		libvlc_time_t t = libvlc_media_player_get_time((libvlc_media_player_t*)vlcmedia);
                return FTimespan::FromMilliseconds(t);
        }
        else
        {
                return FTimespan::Zero();
        }
}

const TArray<IMediaTrackRef>& FVlcMediaPlayer::GetTracks() const
{
	return Tracks;
}


bool FVlcMediaPlayer::IsLooping() const 
{
	if(!vlcmedia)return false;
        if (MediaState == EMediaState::Prepared || MediaState == EMediaState::Started ||
                MediaState == EMediaState::Paused || MediaState == EMediaState::Stopped ||
                MediaState == EMediaState::PlaybackCompleted)
        {
                return false; // TODO todo xxx
        }
        else
        {
                return false;
        }
}


bool FVlcMediaPlayer::IsPaused() const
{
	if(!vlcmedia)return false;
	// xxxi TODO todo unsure exactly the right way to do this
	return !IsPlaying();
	// return MediaState == EMediaState::Paused;
}


bool FVlcMediaPlayer::IsPlaying() const
{
	if(!vlcmedia)return false;
	if(MediaState != EMediaState::Error) {
		return libvlc_media_player_is_playing((libvlc_media_player_t*)vlcmedia) ? true : false;
	}
	return false;
}

bool FVlcMediaPlayer::IsReady() const
{
	if(!vlcmedia)return false;
        return
                MediaState == EMediaState::Prepared ||
                MediaState == EMediaState::Started ||
                MediaState == EMediaState::Paused ||
                MediaState == EMediaState::PlaybackCompleted;
}

bool FVlcMediaPlayer::Open( const FString& Url )
{
	if(vlcmedia)return false; // xxx could close prev

	if(Url.IsEmpty())
	{
		printf("vlc did not get an url\n");
		return false;
	}
	MediaUrl = Url;

	char buffer[1024];
	const TCHAR* chars = *Url;
	sprintf(buffer,"%ls",chars);

	libvlc_media_t *m = libvlc_media_new_path((libvlc_instance_t*)vlchandle, buffer);
	vlcmedia = libvlc_media_player_new_from_media(m);
	libvlc_media_release(m);
	libvlc_video_set_callbacks((libvlc_media_player_t*)vlcmedia, vlc_lock, vlc_unlock, vlc_display, vlccontext);
	libvlc_video_set_format((libvlc_media_player_t*)vlcmedia, "RV32", VIDEOWIDTH, VIDEOHEIGHT, VIDEOWIDTH*4);

	MediaState = EMediaState::Initialized;

	Tracks.Add(MakeShareable(new VideoTrack(*this,Tracks.Num())));
	Tracks.Add(MakeShareable(new AudioTrack(*this,Tracks.Num())));
	OpenedEvent.Broadcast(MediaUrl);
	MediaState = EMediaState::Prepared;
	return true;
}

void FVlcMediaPlayer::Play() {
	if(!vlcmedia)return;
	libvlc_media_player_play((libvlc_media_player_t*)vlcmedia);
}

void FVlcMediaPlayer::Stop() {
	if(!vlcmedia) return;
	libvlc_media_player_stop((libvlc_media_player_t*)vlcmedia);
}

bool FVlcMediaPlayer::Open( const TSharedRef<TArray<uint8>>& Buffer, const FString& OriginalUrl )
{
	printf("vlcplayer: tried to open the wrong way");
	return false;
}


bool FVlcMediaPlayer::Seek( const FTimespan& Time )
{
    if(!vlcmedia)return false;
	if(MediaState != EMediaState::Error)
	{
		libvlc_time_t t = Time.GetMilliseconds(); // not GetTotal? 
		libvlc_media_player_set_time((libvlc_media_player_t*)vlcmedia,t);
		return true;
	}
	return false;
}


bool FVlcMediaPlayer::SetLooping( bool Looping )
{
	// TODO xxx no reason why we can't support this
	if(!vlcmedia)return false;
	return false;
}

bool FVlcMediaPlayer::SetRate(float Rate)
{
	printf("vlc set the rate to %f\n",Rate);
	bool success = libvlc_media_player_set_rate((libvlc_media_player_t*)vlcmedia,Rate) == -1 ? false : true;
	GetRate();
	if(!success) {
		printf("vlc failed to set rate\n");
		return false;
	}

	// xxx for some reason it auto plays

        switch (MediaState) {
        case EMediaState::Prepared:
                if (1.0f == Rate) {
                        Play();
                        MediaState = EMediaState::Started;
                } else if (0.0f == Rate) {
                        Stop();
                        MediaState = EMediaState::Paused;
                }
                break;
        case EMediaState::Paused:
                if (1.0f == Rate) {
                        Play();
                        MediaState = EMediaState::Started;
                }
                break;
        case EMediaState::PlaybackCompleted:
                if (1.0f == Rate) {
                        Play();
                        MediaState = EMediaState::Started;
                }
                break;
	default:
		printf("set rate had an issue with current state\n");
		return false;
        }
	return true;
}


bool FVlcMediaPlayer::Tick( float DeltaTime )
{
    if(IsReady() && IsPlaying())
    {
        for (IMediaTrackRef& track : Tracks)
        {
            MediaTrack &TrackToTick = static_cast<MediaTrack&>(*track);
            // TrackToTick.TickInRenderThread() ?
            TrackToTick.Tick(DeltaTime);
        }
    }
    return true;
}

/*
typedef TWeakPtr<IMediaTrack, ESPMode::ThreadSafe> IMediaTrackWeakPtr;

void FVlcMediaPlayer::Tick(float DeltaTime)
{
        for (IMediaTrackRef track : Tracks)
        {
                MediaTrack & TrackToTick = static_cast<MediaTrack&>(*track);
                if (TrackToTick.TickInRenderThread())
                {
                        ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
                                RenderTickMediaTrack, IMediaTrackWeakPtr, Track, track, float, DeltaT, DeltaTime,
                                {
                                        IMediaTrackPtr t = Track.Pin();
                                        if (t.IsValid())
                                        {
                                                StaticCastSharedPtr<MediaTrack>(t)->Tick(DeltaT);
                                        }
                                });
                }
                else
                {
                        TrackToTick.Tick(DeltaTime);
                }
        }
}

TStatId FVlcMediaPlayer::GetStatId() const
{
        RETURN_QUICK_DECLARE_CYCLE_STAT(FVlcMediaPlayer, STATGROUP_Tickables);
}

bool FVlcMediaPlayer::IsTickable() const
{
        return true;
}
*/

