// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VlcMediaCallbacks.h"
#include "VlcMediaPrivate.h"

#include "IMediaAudioSample.h"
#include "IMediaOptions.h"
#include "IMediaTextureSample.h"
#include "MediaSamples.h"

#include "Vlc.h"
#include "VlcMediaAudioSample.h"
#include "VlcMediaTextureSample.h"


/* FVlcMediaOutput structors
 *****************************************************************************/

FVlcMediaCallbacks::FVlcMediaCallbacks()
	: AudioChannels(0)
	, AudioSampleFormat(EMediaAudioSampleFormat::Int16)
	, AudioSamplePool(new FVlcMediaAudioSamplePool)
	, AudioSampleRate(0)
	, AudioSampleSize(0)
	, CurrentTime(FTimespan::Zero())
	, Player(nullptr)
	, Samples(new FMediaSamples)
	, VideoBufferDim(FIntPoint::ZeroValue)
	, VideoBufferStride(0)
	, VideoFrameDuration(FTimespan::Zero())
	, VideoOutputDim(FIntPoint::ZeroValue)
	, VideoPreviousTime(FTimespan::MinValue())
	, VideoSampleFormat(EMediaTextureSampleFormat::CharAYUV)
	, VideoSamplePool(new FVlcMediaTextureSamplePool)
{ }


FVlcMediaCallbacks::~FVlcMediaCallbacks()
{
	Shutdown();

	delete AudioSamplePool;
	AudioSamplePool = nullptr;

	delete Samples;
	Samples = nullptr;

	delete VideoSamplePool;
	VideoSamplePool = nullptr;
}


/* FVlcMediaOutput interface
 *****************************************************************************/

IMediaSamples& FVlcMediaCallbacks::GetSamples()
{
	return *Samples;
}


void FVlcMediaCallbacks::Initialize(FLibvlcMediaPlayer& InPlayer)
{
	Shutdown();

	Player = &InPlayer;

	// register callbacks
	FVlc::AudioSetFormatCallbacks(
		Player,
		&FVlcMediaCallbacks::StaticAudioSetupCallback,
		&FVlcMediaCallbacks::StaticAudioCleanupCallback
	);

	FVlc::AudioSetCallbacks(
		Player,
		&FVlcMediaCallbacks::StaticAudioPlayCallback,
		&FVlcMediaCallbacks::StaticAudioPauseCallback,
		&FVlcMediaCallbacks::StaticAudioResumeCallback,
		&FVlcMediaCallbacks::StaticAudioFlushCallback,
		&FVlcMediaCallbacks::StaticAudioDrainCallback,
		this
	);

	FVlc::VideoSetFormatCallbacks(
		Player,
		&FVlcMediaCallbacks::StaticVideoSetupCallback,
		&FVlcMediaCallbacks::StaticVideoCleanupCallback
	);

	FVlc::VideoSetCallbacks(
		Player,
		&FVlcMediaCallbacks::StaticVideoLockCallback,
		&FVlcMediaCallbacks::StaticVideoUnlockCallback,
		&FVlcMediaCallbacks::StaticVideoDisplayCallback,
		this
	);
}


void FVlcMediaCallbacks::Shutdown()
{
	if (Player == nullptr)
	{
		return;
	}

	// unregister callbacks
	FVlc::AudioSetCallbacks(Player, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	FVlc::AudioSetFormatCallbacks(Player, nullptr, nullptr);

	FVlc::VideoSetCallbacks(Player, nullptr, nullptr, nullptr, nullptr);
	FVlc::VideoSetFormatCallbacks(Player, nullptr, nullptr);

	AudioSamplePool->Reset();
	VideoSamplePool->Reset();

	CurrentTime = FTimespan::Zero();
	Player = nullptr;
}


/* FVlcMediaOutput static functions
*****************************************************************************/

void FVlcMediaCallbacks::StaticAudioCleanupCallback(void* Opaque)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("Callbacks %llx: StaticAudioCleanupCallback"), Opaque);
}


void FVlcMediaCallbacks::StaticAudioDrainCallback(void* Opaque)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("Callbacks %llx: StaticAudioDrainCallback"), Opaque);
}


void FVlcMediaCallbacks::StaticAudioFlushCallback(void* Opaque, int64 Timestamp)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("Callbacks %llx: StaticAudioFlushCallback"), Opaque);
}


void FVlcMediaCallbacks::StaticAudioPauseCallback(void* Opaque, int64 Timestamp)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("Callbacks %llx: StaticAudioPauseCallback (Timestamp = %i)"), Opaque, Timestamp);

	// do nothing; pausing is handled in Update
}


void FVlcMediaCallbacks::StaticAudioPlayCallback(void* Opaque, void* Samples, uint32 Count, int64 Timestamp)
{
	auto Callbacks = (FVlcMediaCallbacks*)Opaque;

	if (Callbacks == nullptr)
	{
		return;
	}

	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("Callbacks %llx: StaticAudioPlayCallback (Count = %i, Timestamp = %i, Queue = %i)"),
		Opaque,
		Count,
		Timestamp,
		Callbacks->Samples->NumAudio()
	);

	// create & add sample to queue
	auto AudioSample = Callbacks->AudioSamplePool->AcquireShared();

	const FTimespan Delay = FTimespan::FromMicroseconds(FVlc::Delay(Timestamp));
	const FTimespan Duration = FTimespan::FromMicroseconds((Count * 1000000) / Callbacks->AudioSampleRate);
	const SIZE_T SamplesSize = Count * Callbacks->AudioSampleSize * Callbacks->AudioChannels;

	if (AudioSample->Initialize(
		Samples,
		SamplesSize,
		Count,
		Callbacks->AudioChannels,
		Callbacks->AudioSampleFormat,
		Callbacks->AudioSampleRate,
		Callbacks->CurrentTime + Delay,
		Duration))
	{
		Callbacks->Samples->AddAudio(AudioSample);
	}
}


void FVlcMediaCallbacks::StaticAudioResumeCallback(void* Opaque, int64 Timestamp)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("Callbacks %llx: StaticAudioResumeCallback (Timestamp = %i)"), Opaque, Timestamp);

	// do nothing; resuming is handled in Update
}


int FVlcMediaCallbacks::StaticAudioSetupCallback(void** Opaque, ANSICHAR* Format, uint32* Rate, uint32* Channels)
{
	auto Callbacks = *(FVlcMediaCallbacks**)Opaque;

	if (Callbacks == nullptr)
	{
		return -1;
	}

	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("Callbacks %llx: StaticAudioSetupCallback (Format = %s, Rate = %i, Channels = %i)"),
		Opaque,
		ANSI_TO_TCHAR(Format),
		*Rate,
		*Channels
	);

	// setup audio format
	if (*Channels > 8)
	{
		*Channels = 8;
	}

	if (FMemory::Memcmp(Format, "S8  ", 4) == 0)
	{
		Callbacks->AudioSampleFormat = EMediaAudioSampleFormat::Int8;
		Callbacks->AudioSampleSize = 1;
	}
	else if (FMemory::Memcmp(Format, "S16N", 4) == 0)
	{
		Callbacks->AudioSampleFormat = EMediaAudioSampleFormat::Int16;
		Callbacks->AudioSampleSize = 2;
	}
	else if (FMemory::Memcmp(Format, "S32N", 4) == 0)
	{
		Callbacks->AudioSampleFormat = EMediaAudioSampleFormat::Int32;
		Callbacks->AudioSampleSize = 4;
	}
	else if (FMemory::Memcmp(Format, "FL32", 4) == 0)
	{
		Callbacks->AudioSampleFormat = EMediaAudioSampleFormat::Float;
		Callbacks->AudioSampleSize = 4;
	}
	else if (FMemory::Memcmp(Format, "FL64", 4) == 0)
	{
		Callbacks->AudioSampleFormat = EMediaAudioSampleFormat::Double;
		Callbacks->AudioSampleSize = 8;
	}
	else if (FMemory::Memcmp(Format, "U8  ", 4) == 0)
	{
		// unsigned integer fall back
		FMemory::Memcpy(Format, "S8  ", 4);
		Callbacks->AudioSampleFormat = EMediaAudioSampleFormat::Int8;
		Callbacks->AudioSampleSize = 1;
	}
	else
	{
		// unsupported format fall back
		FMemory::Memcpy(Format, "S16N", 4);
		Callbacks->AudioSampleFormat = EMediaAudioSampleFormat::Int16;
		Callbacks->AudioSampleSize = 2;
	}

	Callbacks->AudioChannels = *Channels;
	Callbacks->AudioSampleRate = *Rate;

	return 0;
}


void FVlcMediaCallbacks::StaticVideoCleanupCallback(void *Opaque)
{
	// do nothing
}


void FVlcMediaCallbacks::StaticVideoDisplayCallback(void* Opaque, void* Picture)
{
	auto Callbacks = (FVlcMediaCallbacks*)Opaque;
	auto VideoSample = (FVlcMediaTextureSample*)Picture;

	if ((Callbacks == nullptr) || (VideoSample == nullptr))
	{
		return;
	}

	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("Callbacks %llx: StaticVideoDisplayCallback (CurrentTime = %s, Queue = %i)"),
		Opaque, *Callbacks->CurrentTime.ToString(),
		Callbacks->Samples->NumVideoSamples()
	);

	VideoSample->SetTime(Callbacks->CurrentTime);

	// add sample to queue
	Callbacks->Samples->AddVideo(Callbacks->VideoSamplePool->ToShared(VideoSample));
}


void* FVlcMediaCallbacks::StaticVideoLockCallback(void* Opaque, void** Planes)
{
	auto Callbacks = (FVlcMediaCallbacks*)Opaque;
	check(Callbacks != nullptr);

	FMemory::Memzero(Planes, FVlc::MaxPlanes * sizeof(void*));

	// skip if already processed
	if (Callbacks->VideoPreviousTime == Callbacks->CurrentTime)
	{
		// VLC currently requires a valid buffer or it will crash
		Planes[0] = FMemory::Malloc(Callbacks->VideoBufferStride * Callbacks->VideoBufferDim.Y, 32);
		return nullptr;
	}

	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("Callbacks %llx: StaticVideoLockCallback (CurrentTime = %s)"),
		Opaque,
		*Callbacks->CurrentTime.ToString()
	);

	// create & initialize video sample
	auto VideoSample = Callbacks->VideoSamplePool->Acquire();

	if (VideoSample == nullptr)
	{
		// VLC currently requires a valid buffer or it will crash
		Planes[0] = FMemory::Malloc(Callbacks->VideoBufferStride * Callbacks->VideoBufferDim.Y, 32);
		return nullptr;
	}

	if (!VideoSample->Initialize(
		Callbacks->VideoBufferDim,
		Callbacks->VideoOutputDim,
		Callbacks->VideoSampleFormat,
		Callbacks->VideoBufferStride,
		Callbacks->VideoFrameDuration))
	{
		// VLC currently requires a valid buffer or it will crash
		Planes[0] = FMemory::Malloc(Callbacks->VideoBufferStride * Callbacks->VideoBufferDim.Y, 32);
		return nullptr;
	}

	Callbacks->VideoPreviousTime = Callbacks->CurrentTime;
	Planes[0] = VideoSample->GetMutableBuffer();

	return VideoSample; // passed as Picture into unlock & display callbacks

}


unsigned FVlcMediaCallbacks::StaticVideoSetupCallback(void** Opaque, char* Chroma, unsigned* Width, unsigned* Height, unsigned* Pitches, unsigned* Lines)
{
	auto Callbacks = *(FVlcMediaCallbacks**)Opaque;
	
	if (Callbacks == nullptr)
	{
		return 0;
	}

	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("Callbacks %llx: StaticVideoSetupCallback (Chroma = %s, Dim = %ix%i)"),
		Opaque,
		ANSI_TO_TCHAR(Chroma),
		*Width,
		*Height
	);

	// get video output size
	if (FVlc::VideoGetSize(Callbacks->Player, 0, (uint32*)&Callbacks->VideoOutputDim.X, (uint32*)&Callbacks->VideoOutputDim.Y) != 0)
	{
		Callbacks->VideoBufferDim = FIntPoint::ZeroValue;
		Callbacks->VideoOutputDim = FIntPoint::ZeroValue;
		Callbacks->VideoBufferStride = 0;

		return 0;
	}

	if (Callbacks->VideoOutputDim.GetMin() <= 0)
	{
		return 0;
	}

	// determine decoder & sample formats
	Callbacks->VideoBufferDim = FIntPoint(*Width, *Height);

	if (FCStringAnsi::Stricmp(Chroma, "AYUV") == 0)
	{
		Callbacks->VideoSampleFormat = EMediaTextureSampleFormat::CharAYUV;
		Callbacks->VideoBufferStride = *Width * 4;
	}
	else if (FCStringAnsi::Stricmp(Chroma, "RV32") == 0)
	{
		Callbacks->VideoSampleFormat = EMediaTextureSampleFormat::CharBGRA;
		Callbacks->VideoBufferStride = *Width * 4;
	}
	else if ((FCStringAnsi::Stricmp(Chroma, "UYVY") == 0) ||
		(FCStringAnsi::Stricmp(Chroma, "Y422") == 0) ||
		(FCStringAnsi::Stricmp(Chroma, "UYNV") == 0) ||
		(FCStringAnsi::Stricmp(Chroma, "HDYC") == 0))
	{
		Callbacks->VideoSampleFormat = EMediaTextureSampleFormat::CharUYVY;
		Callbacks->VideoBufferStride = *Width * 2;
	}
	else if ((FCStringAnsi::Stricmp(Chroma, "YUY2") == 0) ||
		(FCStringAnsi::Stricmp(Chroma, "V422") == 0) ||
		(FCStringAnsi::Stricmp(Chroma, "YUYV") == 0))
	{
		Callbacks->VideoSampleFormat = EMediaTextureSampleFormat::CharYUY2;
		Callbacks->VideoBufferStride = *Width * 2;
	}
	else if (FCStringAnsi::Stricmp(Chroma, "YVYU") == 0)
	{
		Callbacks->VideoSampleFormat = EMediaTextureSampleFormat::CharYVYU;
		Callbacks->VideoBufferStride = *Width * 2;
	}
	else
	{
		// reconfigure output for natively supported format
		FLibvlcChromaDescription* ChromaDescr = FVlc::FourccGetChromaDescription(*(FLibvlcFourcc*)Chroma);

		if (ChromaDescr->PlaneCount == 0)
		{
			return 0;
		}

		if (ChromaDescr->PlaneCount > 1)
		{
			FMemory::Memcpy(Chroma, "YUY2", 4);

			Callbacks->VideoBufferDim = FIntPoint(Align(Callbacks->VideoOutputDim.X, 16) / 2, Align(Callbacks->VideoOutputDim.Y, 16));
			Callbacks->VideoSampleFormat = EMediaTextureSampleFormat::CharYUY2;
			Callbacks->VideoBufferStride = Callbacks->VideoBufferDim.X * 4;
			*Height = Callbacks->VideoBufferDim.Y;
		}
		else
		{
			FMemory::Memcpy(Chroma, "RV32", 4);

			Callbacks->VideoBufferDim = Callbacks->VideoOutputDim;
			Callbacks->VideoSampleFormat = EMediaTextureSampleFormat::CharBGRA;
			Callbacks->VideoBufferStride = Callbacks->VideoBufferDim.X * 4;
		}
	}

	// get other video properties
	Callbacks->VideoFrameDuration = FTimespan::FromSeconds(1.0 / FVlc::MediaPlayerGetFps(Callbacks->Player));

	// initialize decoder
	Lines[0] = Callbacks->VideoBufferDim.Y;
	Pitches[0] = Callbacks->VideoBufferStride;

	return 1;
}


void FVlcMediaCallbacks::StaticVideoUnlockCallback(void* Opaque, void* Picture, void* const* Planes)
{
	if ((Opaque != nullptr) && (Picture != nullptr))
	{
		UE_LOG(LogVlcMedia, VeryVerbose, TEXT("Callbacks %llx: StaticVideoUnlockCallback"), Opaque);
	}

	// discard temporary buffer for VLC crash workaround
	if ((Picture == nullptr) && (Planes != nullptr) && (Planes[0] != nullptr))
	{
		FMemory::Free(Planes[0]);
	}
}
