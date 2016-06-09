// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPCH.h"
#include "Vlc.h"
#include "VlcMediaOutput.h"


/* FVlcMediaOutput structors
*****************************************************************************/

FVlcMediaOutput::FVlcMediaOutput()
	: AudioSink(nullptr)
	, CaptionSink(nullptr)
	, CurrentTime(FTimespan::Zero())
	, Player(nullptr)
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

	if (AudioSink != nullptr)
	{
		AudioSink->ShutdownAudioSink();
	}

	AudioSink = Sink;
	SetupAudioOutput();
}


void FVlcMediaOutput::SetCaptionSink(IMediaStringSink* Sink)
{
	FScopeLock Lock(&CriticalSection);

	if (CaptionSink != nullptr)
	{
		CaptionSink->ShutdownStringSink();
	}

	CaptionSink = Sink;
	SetupCaptionOutput();
}


void FVlcMediaOutput::SetImageSink(IMediaTextureSink* Sink)
{
	// not supported
}


void FVlcMediaOutput::SetVideoSink(IMediaTextureSink* Sink)
{
	FScopeLock Lock(&CriticalSection);

	if (VideoSink != nullptr)
	{
		VideoSink->ShutdownTextureSink();
	}

	VideoSink = Sink;
	SetupVideoOutput();
}


/* FVlcMediaOutput implementation
*****************************************************************************/

void FVlcMediaOutput::SetupAudioOutput()
{
	if (Player == nullptr)
	{
		return;
	}

	// @todo gmp: vlc: fix audio specs
	if ((AudioSink != nullptr) && AudioSink->InitializeAudioSink(2, 44100))
	{
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
		FVlc::AudioSetCallbacks(Player, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	}
}


void FVlcMediaOutput::SetupCaptionOutput()
{
	// @todo gmp: vlc: implement caption tracks
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
			&FVlcMediaOutput::StaticVideoFormatCallback,
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

void FVlcMediaOutput::StaticAudioDrainCallback(void* Opaque)
{
	// do nothing
}


void FVlcMediaOutput::StaticAudioFlushCallback(void* Opaque, int64 Timestamp)
{
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
	auto Output = (FVlcMediaOutput*)Opaque;

	if (Output == nullptr)
	{
		return;
	}

	FScopeLock Lock(&Output->CriticalSection);
	IMediaAudioSink* AudioSink = Output->AudioSink;

	if (AudioSink != nullptr)
	{
		AudioSink->PlayAudioSink((uint8*)Samples, Count, FTimespan::FromMicroseconds(Timestamp));
	}
}


void FVlcMediaOutput::StaticAudioResumeCallback(void* Opaque, int64 Timestamp)
{
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


void FVlcMediaOutput::StaticVideoCleanupCallback(void *Opaque)
{
	auto Output = (FVlcMediaOutput*)Opaque;

	if ((Output == nullptr) || (Output->VideoSink == nullptr))
	{
		return;
	}

	Output->VideoSink->ShutdownTextureSink();
}


void FVlcMediaOutput::StaticVideoDisplayCallback(void* Opaque, void* /*Picture*/)
{
	auto Output = (FVlcMediaOutput*)Opaque;

	if (Output == nullptr)
	{
		return;
	}

	FScopeLock Lock(&Output->CriticalSection);
	IMediaTextureSink* VideoSink = Output->VideoSink;

	if (VideoSink != nullptr)
	{
		VideoSink->DisplayTextureSinkBuffer(Output->CurrentTime);
	}
}


unsigned FVlcMediaOutput::StaticVideoFormatCallback(void** Opaque, char* Chroma, unsigned* Width, unsigned* Height, unsigned* Pitches, unsigned* Lines)
{
	auto Output = *(FVlcMediaOutput**)Opaque;

	if ((Output == nullptr) || (Output->VideoSink == nullptr))
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
			FMemory::Memcpy(Chroma, "UYVY", 4);
			SinkFormat = EMediaTextureSinkFormat::CharUYVY;
			Pitches[0] = *Width * 2;
			Lines[0] = *Height;
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
	if (!Output->VideoSink->InitializeTextureSink(FIntPoint(*Width, *Height), SinkFormat, EMediaTextureSinkMode::Buffered))
	{
		return 0;
	}

	Output->VideoDimensions = FIntPoint(*Width, *Height);

	return 1;
}


void* FVlcMediaOutput::StaticVideoLockCallback(void* Opaque, void** Planes)
{
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
		*Planes = VideoSink->AcquireTextureSinkBuffer();
	}

	if (*Planes == nullptr)
	{
		// VLC currently requires a valid buffer or it will crash, but the
		// sink may not be ready yet, so we create a temporary buffer here
		*Planes = FMemory::Malloc(Output->VideoDimensions.X * Output->VideoDimensions.Y * 4, 32);

		return *Planes;
	}

	return nullptr;
}


void FVlcMediaOutput::StaticVideoUnlockCallback(void* Opaque, void* Picture, void* const* /*Planes*/)
{
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
