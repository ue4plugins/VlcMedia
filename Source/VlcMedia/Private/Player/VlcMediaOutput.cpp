// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"
#include "Vlc.h"
#include "VlcMediaOutput.h"


/* FVlcMediaOutput structors
*****************************************************************************/

FVlcMediaOutput::FVlcMediaOutput()
	: AudioSink(nullptr)
	, CaptionSink(nullptr)
	, Player(nullptr)
	, VideoSink(nullptr)
{ }


/* FVlcMediaOutput interface
*****************************************************************************/

void FVlcMediaOutput::Initialize(FLibvlcMediaPlayer& InPlayer)
{
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

	if ((VideoSink != nullptr) && VideoSink->InitializeTextureSink(FIntPoint(FVlc::VideoGetWidth(Player), FVlc::VideoGetHeight(Player)), EMediaTextureSinkMode::Buffered))
	{
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
		FVlc::VideoSetCallbacks(Player, nullptr, nullptr, nullptr, nullptr);
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
		VideoSink->DisplayTextureSinkBuffer();
	}
}


void* FVlcMediaOutput::StaticVideoLockCallback(void* Opaque, void** Planes)
{
	auto Output = (FVlcMediaOutput*)Opaque;

	if (Output == nullptr)
	{
		return nullptr;
	}

	FScopeLock Lock(&Output->CriticalSection);
	IMediaTextureSink* VideoSink = Output->VideoSink;

	if (VideoSink != nullptr)
	{
		*Planes = VideoSink->AcquireTextureSinkBuffer();
	}

	return nullptr;
}


void FVlcMediaOutput::StaticVideoUnlockCallback(void* Opaque, void* /*Picture*/, void* const* Planes)
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
		VideoSink->ReleaseTextureSinkBuffer();
	}
}
