// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPCH.h"
#include "Vlc.h"
#include "VlcMediaOutput.h"


/* FVlcMediaOutput structors
*****************************************************************************/

FVlcMediaOutput::FVlcMediaOutput()
	: AudioSink(nullptr)
	, CaptionSink(nullptr)
	, Player(nullptr)
	, ResumeOrigin(0)
	, ResumeTime(FTimespan::Zero())
	, SubtitleSink(nullptr)
	, VideoSink(nullptr)
{ }


/* FVlcMediaOutput interface
*****************************************************************************/

void FVlcMediaOutput::Initialize(FLibvlcMediaPlayer& InPlayer)
{
	Shutdown();

	Player = &InPlayer;

	SetupAudioOutput();
	SetupCaptionOutput();
	SetupVideoOutput();
}


void FVlcMediaOutput::Resume(FTimespan InResumeTime)
{
	UE_LOG(LogVlcMedia, Verbose, TEXT("Resuming output at %s"), *InResumeTime.ToString());

	FScopeLock Lock(&CriticalSection);

	ResumeOrigin = FVlc::Clock();
	ResumeTime = InResumeTime;

	if (AudioSink != nullptr)
	{
		AudioSink->ResumeAudioSink();
	}
}


void FVlcMediaOutput::Shutdown()
{
	if (Player != nullptr)
	{
		FVlc::AudioSetCallbacks(Player, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
		FVlc::VideoSetCallbacks(Player, nullptr, nullptr, nullptr, nullptr);
		Player = nullptr;
	}
}


/* IMediaOutput interface
*****************************************************************************/

void FVlcMediaOutput::SetAudioSink(IMediaAudioSink* Sink)
{
	FScopeLock Lock(&CriticalSection);

	if (Sink != AudioSink)
	{
		if (AudioSink != nullptr)
		{
			AudioSink->ShutdownAudioSink();
		}

		AudioSink = Sink;
		SetupAudioOutput();
	}
}


void FVlcMediaOutput::SetCaptionSink(IMediaStringSink* Sink)
{
	FScopeLock Lock(&CriticalSection);

	if (Sink != CaptionSink)
	{
		if (CaptionSink != nullptr)
		{
			CaptionSink->ShutdownStringSink();
		}

		CaptionSink = Sink;
		SetupCaptionOutput();
	}
}

/*
void FVlcMediaOutput::SetSubtitleSink(IMediaStringSink* Sink)
{
	FScopeLock Lock(&CriticalSection);

	if (Sink != SubtitleSink)
	{
		if (SubtitleSink != nullptr)
		{
			SubtitleSink->ShutdownStringSink();
		}

		SubtitleSink = Sink;
		SetupSubtitleOutput();
	}
}*/


void FVlcMediaOutput::SetVideoSink(IMediaTextureSink* Sink)
{
	FScopeLock Lock(&CriticalSection);

	if (Sink != VideoSink)
	{
		if (VideoSink != nullptr)
		{
			VideoSink->ShutdownTextureSink();
		}

		VideoSink = Sink;
		SetupVideoOutput();
	}
}


/* FVlcMediaOutput implementation
*****************************************************************************/

void FVlcMediaOutput::SetupAudioOutput()
{
	if (Player == nullptr)
	{
		return;
	}

	if (AudioSink != nullptr)
	{
		// register callbacks
		FVlc::AudioSetFormatCallbacks(
			Player,
			&FVlcMediaOutput::StaticAudioSetupCallback,
			&FVlcMediaOutput::StaticAudioCleanupCallback
		);

		FVlc::AudioSetCallbacks(
			Player,
			&FVlcMediaOutput::StaticAudioPlayCallback,
			&FVlcMediaOutput::StaticAudioPauseCallback,
			&FVlcMediaOutput::StaticAudioResumeCallback,
			&FVlcMediaOutput::StaticAudioFlushCallback,
			&FVlcMediaOutput::StaticAudioDrainCallback,
			this
		);
	}
	else
	{
		// unregister callbacks
		FVlc::AudioSetCallbacks(Player, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
		FVlc::AudioSetFormatCallbacks(Player, nullptr, nullptr);
	}
}


void FVlcMediaOutput::SetupCaptionOutput()
{
	// @todo gmp: vlc: implement caption tracks
}


void FVlcMediaOutput::SetupSubtitleOutput()
{
	// @todo gmp: vlc: implement subtitle tracks
}


void FVlcMediaOutput::SetupVideoOutput()
{
	if (Player == nullptr)
	{
		return;
	}

	if (VideoSink != nullptr)
	{
		// register callbacks
		FVlc::VideoSetFormatCallbacks(
			Player,
			&FVlcMediaOutput::StaticVideoSetupCallback,
			&FVlcMediaOutput::StaticVideoCleanupCallback
		);

		FVlc::VideoSetCallbacks(
			Player,
			&FVlcMediaOutput::StaticVideoLockCallback,
			&FVlcMediaOutput::StaticVideoUnlockCallback,
			&FVlcMediaOutput::StaticVideoDisplayCallback,
			this
		);
	}
	else
	{
		// unregister callbacks
		FVlc::VideoSetCallbacks(Player, nullptr, nullptr, nullptr, nullptr);
		FVlc::VideoSetFormatCallbacks(Player, nullptr, nullptr);
	}
}


/* FVlcMediaOutput static functions
*****************************************************************************/

void FVlcMediaOutput::StaticAudioCleanupCallback(void* Opaque)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("StaticAudioCleanupCallback"));

	auto Output = (FVlcMediaOutput*)Opaque;

	if (Output == nullptr)
	{
		return;
	}

	FScopeLock Lock(&Output->CriticalSection);
	IMediaAudioSink* AudioSink = Output->AudioSink;

	if (AudioSink != nullptr)
	{
		AudioSink->ShutdownAudioSink();
	}
}


void FVlcMediaOutput::StaticAudioDrainCallback(void* Opaque)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("StaticAudioDrainCallback"));
}


void FVlcMediaOutput::StaticAudioFlushCallback(void* Opaque, int64 Timestamp)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("StaticAudioFlushCallback"));

	auto Output = (FVlcMediaOutput*)Opaque;

	if (Output == nullptr)
	{
		return;
	}

	FScopeLock Lock(&Output->CriticalSection);
	IMediaAudioSink* AudioSink = Output->AudioSink;

	if (AudioSink != nullptr)
	{
		AudioSink->FlushAudioSink();
	}
}


void FVlcMediaOutput::StaticAudioPauseCallback(void* Opaque, int64 Timestamp)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("StaticAudioPauseCallback"));

	auto Output = (FVlcMediaOutput*)Opaque;

	if (Output == nullptr)
	{
		return;
	}

	FScopeLock Lock(&Output->CriticalSection);
	IMediaAudioSink* AudioSink = Output->AudioSink;

	if (AudioSink != nullptr)
	{
		AudioSink->PauseAudioSink();
	}
}


void FVlcMediaOutput::StaticAudioPlayCallback(void* Opaque, void* Samples, uint32 Count, int64 Timestamp)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("StaticAudioPlayCallback: Count=%i"), Count);

	auto Output = (FVlcMediaOutput*)Opaque;

	if (Output == nullptr)
	{
		return;
	}

	FScopeLock Lock(&Output->CriticalSection);
	IMediaAudioSink* AudioSink = Output->AudioSink;

	if (AudioSink != nullptr)
	{
		AudioSink->PlayAudioSink((uint8*)Samples, Count * AudioSink->GetAudioSinkChannels() * sizeof(int16), Output->TimestampToTimespan(Timestamp));
	}
}


void FVlcMediaOutput::StaticAudioResumeCallback(void* Opaque, int64 Timestamp)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("StaticAudioResumeCallback"));

	auto Output = (FVlcMediaOutput*)Opaque;

	if (Output == nullptr)
	{
		return;
	}

	FScopeLock Lock(&Output->CriticalSection);
	IMediaAudioSink* AudioSink = Output->AudioSink;

	if (AudioSink != nullptr)
	{
		AudioSink->ResumeAudioSink();
	}
}


int FVlcMediaOutput::StaticAudioSetupCallback(void** Opaque, ANSICHAR* Format, uint32* Rate, uint32* Channels)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("StaticAudioSetupCallback: Format=%s Rate=%i Channels=%i"), ANSI_TO_TCHAR(Format), *Rate, *Channels);

	auto Output = *(FVlcMediaOutput**)Opaque;

	if ((Output == nullptr) || (Output->VideoSink == nullptr))
	{
		return -1;
	}

	FScopeLock Lock(&Output->CriticalSection);
	IMediaAudioSink* AudioSink = Output->AudioSink;

	if (AudioSink == nullptr)
	{
		return -1;
	}

	// set decoder format
	FMemory::Memcpy(Format, "S16N", 4);

	if ((*Channels != 1) && (*Channels != 2) && (*Channels != 6))
	{
		*Channels = 2;
	}

	if (*Rate != 44100u)
	{
		UE_LOG(LogVlcMedia, Warning, TEXT("Possible loss of audio quality due to sample rate != 44100 Hz"));
	}

	// initialize sink
	if (!AudioSink->InitializeAudioSink(*Channels, *Rate))
	{
		return -1;
	}

	return 0;
}


void FVlcMediaOutput::StaticVideoCleanupCallback(void *Opaque)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("StaticVideoCleanupCallback"));

	auto Output = (FVlcMediaOutput*)Opaque;

	if (Output == nullptr)
	{
		return;
	}

	FScopeLock Lock(&Output->CriticalSection);
	IMediaTextureSink* VideoSink = Output->VideoSink;

	if (VideoSink != nullptr)
	{
		VideoSink->ShutdownTextureSink();
	}
}


void FVlcMediaOutput::StaticVideoDisplayCallback(void* Opaque, void* Picture)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("StaticVideoDisplayCallback"));

	auto Output = (FVlcMediaOutput*)Opaque;

	if (Output == nullptr)
	{
		return;
	}

	FScopeLock Lock(&Output->CriticalSection);
	IMediaTextureSink* VideoSink = Output->VideoSink;

	if (VideoSink != nullptr)
	{
		VideoSink->DisplayTextureSinkBuffer(Output->TimestampToTimespan(FVlc::Clock()));
	}
}


void* FVlcMediaOutput::StaticVideoLockCallback(void* Opaque, void** Planes)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("StaticVideoLockCallback"));

	auto Output = (FVlcMediaOutput*)Opaque;

	if (Output == nullptr)
	{
		return nullptr;
	}

	FMemory::Memzero(Planes, FVlc::MaxPlanes * sizeof(void*));

	FScopeLock Lock(&Output->CriticalSection);
	IMediaTextureSink* VideoSink = Output->VideoSink;

	if (VideoSink != nullptr)
	{
		Planes[0] = VideoSink->AcquireTextureSinkBuffer();
	}

	if (Planes[0] == nullptr)
	{
		// VLC currently requires a valid buffer or it will crash, but the
		// sink may not be ready yet, so we create a temporary buffer here
		Planes[0] = FMemory::Malloc(Output->VideoDimensions.X * Output->VideoDimensions.Y * 4, 32);

		return Planes[0];
	}

	return nullptr;
}


unsigned FVlcMediaOutput::StaticVideoSetupCallback(void** Opaque, char* Chroma, unsigned* Width, unsigned* Height, unsigned* Pitches, unsigned* Lines)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("StaticVideoSetupCallback: Chroma=%s Dim=%ix%i"), ANSI_TO_TCHAR(Chroma), *Width, *Height);

	auto Output = *(FVlcMediaOutput**)Opaque;

	if ((Output == nullptr) || (Output->VideoSink == nullptr))
	{
		return 0;
	}

	FScopeLock Lock(&Output->CriticalSection);
	IMediaTextureSink* VideoSink = Output->VideoSink;

	if (VideoSink == nullptr)
	{
		return 0;
	}

	Output->VideoDimensions = FIntPoint::ZeroValue;

	// determine decoder & sink format
	EMediaTextureSinkFormat SinkFormat;

	if (FCStringAnsi::Stricmp(Chroma, "AYUV") == 0)
	{
		SinkFormat = EMediaTextureSinkFormat::CharAYUV;
		Pitches[0] = *Width * 4;
	}
	else if (FCStringAnsi::Stricmp(Chroma, "UYVY") == 0)
	{
		SinkFormat = EMediaTextureSinkFormat::CharUYVY;
		Pitches[0] = *Width * 2;
	}
	else if ((FCStringAnsi::Stricmp(Chroma, "UYVY") == 0) ||
			(FCStringAnsi::Stricmp(Chroma, "Y422") == 0) ||
			(FCStringAnsi::Stricmp(Chroma, "UYNV") == 0) ||
			(FCStringAnsi::Stricmp(Chroma, "HDYC") == 0))
	{
		SinkFormat = EMediaTextureSinkFormat::CharUYVY;
		Pitches[0] = *Width * 2;
	}
	else if ((FCStringAnsi::Stricmp(Chroma, "YUY2") == 0) ||
			(FCStringAnsi::Stricmp(Chroma, "V422") == 0) ||
			(FCStringAnsi::Stricmp(Chroma, "YUYV") == 0))
	{
		SinkFormat = EMediaTextureSinkFormat::CharYUY2;
		Pitches[0] = *Width * 2;
	}
	else if (FCStringAnsi::Stricmp(Chroma, "YVYU") == 0)
	{
		SinkFormat = EMediaTextureSinkFormat::CharYVYU;
		Pitches[0] = *Width * 2;
	}
	else
	{
		FLibvlcChromaDescription* ChromaDescr = FVlc::FourccGetChromaDescription(*(FLibvlcFourcc*)Chroma);

		if (ChromaDescr->PlaneCount == 0)
		{
			return 0;
		}

		if (ChromaDescr->PlaneCount > 1)
		{
			FMemory::Memcpy(Chroma, "YUY2", 4);
			SinkFormat = EMediaTextureSinkFormat::CharYUY2;
			Pitches[0] = *Width * 2;
		}
		else
		{
			FMemory::Memcpy(Chroma, "RV32", 4);
			SinkFormat = EMediaTextureSinkFormat::CharBGRA;
			Pitches[0] = *Width * 4;
		}
	}

	Lines[0] = *Height;

	// initialize sink
	if (!VideoSink->InitializeTextureSink(FIntPoint(*Width, *Height), SinkFormat, EMediaTextureSinkMode::Buffered))
	{
		return 0;
	}

	Output->VideoDimensions = FIntPoint(*Width, *Height);

	return 1;
}


void FVlcMediaOutput::StaticVideoUnlockCallback(void* Opaque, void* Picture, void* const* /*Planes*/)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("StaticVideoUnlockCallback"));

	auto Output = (FVlcMediaOutput*)Opaque;

	if (Output == nullptr)
	{
		return;
	}

	// update sink
	FScopeLock Lock(&Output->CriticalSection);
	IMediaTextureSink* VideoSink = Output->VideoSink;

	if (VideoSink != nullptr)
	{
		if (Picture != nullptr)
		{
			VideoSink->UpdateTextureSinkBuffer((const uint8*)Picture);
		}
		else
		{
			VideoSink->ReleaseTextureSinkBuffer();
		}
	}

	// free temporary buffer
	if (Picture != nullptr)
	{
		FMemory::Free(Picture);
	}
}
