// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaOutput.h"
#include "VlcMediaPrivate.h"

#include "IMediaAudioSample.h"
#include "IMediaAudioSink.h"
#include "IMediaOptions.h"
#include "IMediaOverlaySink.h"
#include "IMediaTextureSample.h"
#include "IMediaTextureSink.h"

#include "Vlc.h"
#include "VlcMediaAudioSample.h"
#include "VlcMediaTextureSample.h"


/* FVlcMediaOutput structors
 *****************************************************************************/

FVlcMediaOutput::FVlcMediaOutput()
	: AudioChannels(0)
	, AudioSampleFormat(EMediaAudioSampleFormat::Int16)
	, AudioSampleRate(0)
	, AudioSampleSize(0)
	, Player(nullptr)
	, TimeInfo(MakeShareable(new FVlcMediaTimeInfo))
	, VideoBufferDim(FIntPoint::ZeroValue)
	, VideoBufferStride(0)
	, VideoOutputDim(FIntPoint::ZeroValue)
	, VideoPreviousTime(FTimespan::MinValue())
	, VideoSampleFormat(EMediaTextureSampleFormat::CharAYUV)
{ }


/* FVlcMediaOutput interface
 *****************************************************************************/

void FVlcMediaOutput::Initialize(FLibvlcMediaPlayer& InPlayer)
{
	Shutdown();

	Player = &InPlayer;

	SetupAudioOutput();
	SetupOverlayOutput();
	SetupVideoOutput();
}


void FVlcMediaOutput::Shutdown()
{
	if (Player == nullptr)
	{
		return;
	}

	FVlc::AudioSetCallbacks(Player, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	FVlc::VideoSetCallbacks(Player, nullptr, nullptr, nullptr, nullptr);

	Player = nullptr;
	TimeInfo = MakeShareable(new FVlcMediaTimeInfo);

	FlushSinks(true);
}


void FVlcMediaOutput::Update(FTimespan Timecode, FTimespan Time, float Rate)
{
	// update sinks
	if (Rate != TimeInfo->Rate)
	{
		FlushSinks(false);

		if (AudioSinkPtr.IsValid())
		{
			AudioSinkPtr.Pin()->SetAudioSinkRate(Rate);
		}
	}

	// atomically set new time information
	auto NewTimeInfo = new FVlcMediaTimeInfo();
	{
		if (Rate != TimeInfo->Rate)
		{
			NewTimeInfo->StartOffset = Time;
			NewTimeInfo->StartTimecode = Timecode;
		}
		else
		{
			NewTimeInfo->StartOffset = TimeInfo->StartOffset;
			NewTimeInfo->StartTimecode = TimeInfo->StartTimecode;
		}

		NewTimeInfo->Timecode = Timecode;
		NewTimeInfo->Time = Time;
		NewTimeInfo->Rate = Rate;
	}

	TimeInfo = MakeShareable(NewTimeInfo);
}


/* IMediaOutput interface
*****************************************************************************/

bool FVlcMediaOutput::SetAudioNative(bool Enabled)
{
	return false; // not implemented yet
}


void FVlcMediaOutput::SetAudioNativeVolume(float Volume)
{
	// not implemented yet
}


void FVlcMediaOutput::SetAudioSink(TSharedPtr<IMediaAudioSink, ESPMode::ThreadSafe> Sink)
{
	if (Sink != AudioSinkPtr)
	{
		if (AudioSinkPtr.IsValid())
		{
			AudioSinkPtr.Pin()->FlushAudioSink(true);
		}

		AudioSinkPtr = Sink;
		SetupAudioOutput();
	}
}


void FVlcMediaOutput::SetMetadataSink(TSharedPtr<IMediaBinarySink, ESPMode::ThreadSafe> Sink)
{
	// not supported
}


void FVlcMediaOutput::SetOverlaySink(TSharedPtr<IMediaOverlaySink, ESPMode::ThreadSafe> Sink)
{
	if (Sink != OverlaySinkPtr)
	{
		if (OverlaySinkPtr.IsValid())
		{
			OverlaySinkPtr.Pin()->FlushOverlaySink(true);
		}

		OverlaySinkPtr = Sink;
		SetupOverlayOutput();
	}
}


void FVlcMediaOutput::SetVideoSink(TSharedPtr<IMediaTextureSink, ESPMode::ThreadSafe> Sink)
{
	if (Sink != VideoSinkPtr)
	{
		if (VideoSinkPtr.IsValid())
		{
			VideoSinkPtr.Pin()->FlushTextureSink(true);
		}

		VideoSinkPtr = Sink;
		SetupVideoOutput();
	}
}


/* FVlcMediaOutput implementation
*****************************************************************************/

void FVlcMediaOutput::FlushSinks(bool Shutdown)
{
	if (AudioSinkPtr.IsValid())
	{
		AudioSinkPtr.Pin()->FlushAudioSink(Shutdown);
	}

	if (OverlaySinkPtr.IsValid())
	{
		OverlaySinkPtr.Pin()->FlushOverlaySink(Shutdown);
	}

	if (VideoSinkPtr.IsValid())
	{
		VideoSinkPtr.Pin()->FlushTextureSink(Shutdown);
	}
}


void FVlcMediaOutput::SetupAudioOutput()
{
	if (Player == nullptr)
	{
		return;
	}

	auto AudioSink = AudioSinkPtr.Pin();

	if (AudioSink.IsValid())
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

		AudioSink->SetAudioSinkRate(TimeInfo->Rate);
	}
	else
	{
		// unregister callbacks
		FVlc::AudioSetCallbacks(Player, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
		FVlc::AudioSetFormatCallbacks(Player, nullptr, nullptr);
	}
}


void FVlcMediaOutput::SetupOverlayOutput()
{
	if (Player == nullptr)
	{
		return;
	}

	auto OverlaySink = OverlaySinkPtr.Pin();

	if (OverlaySink.IsValid())
	{
		// @todo gmp: vlc: implement caption/subtitle tracks
	}
}


void FVlcMediaOutput::SetupVideoOutput()
{
	if (Player == nullptr)
	{
		return;
	}

	auto VideoSink = VideoSinkPtr.Pin();

	if (VideoSink.IsValid())
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

	auto AudioSink = Output->AudioSinkPtr.Pin();

	if (AudioSink.IsValid())
	{
		AudioSink->FlushAudioSink(true);
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

	auto AudioSink = Output->AudioSinkPtr.Pin();

	if (AudioSink.IsValid())
	{
		AudioSink->FlushAudioSink(false);
	}
}


void FVlcMediaOutput::StaticAudioPauseCallback(void* Opaque, int64 Timestamp)
{
	// do nothing; pausing is handled in Update
}


void FVlcMediaOutput::StaticAudioPlayCallback(void* Opaque, void* Samples, uint32 Count, int64 Timestamp)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("StaticAudioPlayCallback: Count=%i, Timestamp=%i"), Count, Timestamp);

	auto Output = (FVlcMediaOutput*)Opaque;

	if (Output == nullptr)
	{
		return;
	}

	auto AudioSink = Output->AudioSinkPtr.Pin();

	if (!AudioSink.IsValid())
	{
		return;
	}

	auto PinnedTimeInfo = Output->TimeInfo;

	// calculate time code at which the sample should be played
	const FTimespan Delay = FTimespan::FromMicroseconds(FVlc::Delay(Timestamp));
	const FTimespan SampleTime = PinnedTimeInfo->Time + Delay;
	const FTimespan SampleTimecode = PinnedTimeInfo->Timecode + Delay;
	const FTimespan Duration = FTimespan::FromSeconds((double)Count / (double)Output->AudioSampleRate);

	// copy sample buffer
	const SIZE_T BufferSize = Count * Output->AudioSampleSize * Output->AudioChannels;
	void* Buffer = FMemory::Malloc(BufferSize);
	FMemory::Memcpy(Buffer, Samples, BufferSize);
	TSharedPtr<void, ESPMode::ThreadSafe> BufferRef = MakeShareable(Buffer, [](void* Ptr) { FMemory::Free(Ptr); });
		
	// forward sample to audio sink
	TSharedRef<FVlcMediaAudioSample, ESPMode::ThreadSafe> Sample = MakeShareable(
		new FVlcMediaAudioSample(
			BufferRef,
			Count,
			Output->AudioChannels,
			Output->AudioSampleFormat,
			Output->AudioSampleRate,
			SampleTime,
			SampleTimecode,
			Duration
		)
	);

	AudioSink->OnAudioSample(Sample);
}


void FVlcMediaOutput::StaticAudioResumeCallback(void* Opaque, int64 Timestamp)
{
	// do nothing; resuming is handled in Update
}


int FVlcMediaOutput::StaticAudioSetupCallback(void** Opaque, ANSICHAR* Format, uint32* Rate, uint32* Channels)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("StaticAudioSetupCallback: Format=%s Rate=%i Channels=%i"), ANSI_TO_TCHAR(Format), *Rate, *Channels);

	auto Output = *(FVlcMediaOutput**)Opaque;

	if (Output == nullptr)
	{
		return -1;
	}

	auto AudioSink = Output->AudioSinkPtr.Pin();

	if (!AudioSink.IsValid())
	{
		return -1;
	}

	// setup audio format
	if (*Channels > 8)
	{
		*Channels = 8;
	}

	if (FMemory::Memcmp(Format, "S8  ", 4) == 0)
	{
		Output->AudioSampleFormat = EMediaAudioSampleFormat::Int8;
		Output->AudioSampleSize = 1;
	}
	else if (FMemory::Memcmp(Format, "S16N", 4) == 0)
	{
		Output->AudioSampleFormat = EMediaAudioSampleFormat::Int16;
		Output->AudioSampleSize = 2;
	}
	else if (FMemory::Memcmp(Format, "S32N", 4) == 0)
	{
		Output->AudioSampleFormat = EMediaAudioSampleFormat::Int32;
		Output->AudioSampleSize = 4;
	}
	else if (FMemory::Memcmp(Format, "FL32", 4) == 0)
	{
		Output->AudioSampleFormat = EMediaAudioSampleFormat::Float;
		Output->AudioSampleSize = 4;
	}
	else if (FMemory::Memcmp(Format, "FL64", 4) == 0)
	{
		Output->AudioSampleFormat = EMediaAudioSampleFormat::Double;
		Output->AudioSampleSize = 8;
	}
	else if (FMemory::Memcmp(Format, "U8  ", 4) == 0)
	{
		// unsigned integer fall back
		FMemory::Memcpy(Format, "S8  ", 4);
		Output->AudioSampleFormat = EMediaAudioSampleFormat::Int8;
		Output->AudioSampleSize = 1;
	}
	else
	{
		// unsupported format fall back
		FMemory::Memcpy(Format, "S16N", 4);
		Output->AudioSampleFormat = EMediaAudioSampleFormat::Int16;
		Output->AudioSampleSize = 2;
	}

	Output->AudioChannels = *Channels;
	Output->AudioSampleRate = *Rate;

	return 0;
}


void FVlcMediaOutput::StaticVideoCleanupCallback(void *Opaque)
{
	// do nothing
}


void FVlcMediaOutput::StaticVideoDisplayCallback(void* Opaque, void* Picture)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("StaticVideoDisplayCallback"));

	auto Output = (FVlcMediaOutput*)Opaque;
	
	if ((Output == nullptr) || (Picture == nullptr))
	{
		return;
	}

	auto PinnedTimeInfo = Output->TimeInfo;
	auto VideoSink = Output->VideoSinkPtr.Pin();

	// skip if no video sink assigned, or if frame already processed
	if (!VideoSink.IsValid() || (Output->VideoPreviousTime == PinnedTimeInfo->Time))
	{
		FMemory::Free(Picture);

		return;
	}

	Output->VideoPreviousTime = PinnedTimeInfo->Time;

	// calculate time code at which the sample should be displayed
	const FTimespan SampleOffset = (PinnedTimeInfo->Time - PinnedTimeInfo->StartOffset) / PinnedTimeInfo->Rate;
	const FTimespan SampleTimecode = PinnedTimeInfo->StartTimecode + SampleOffset;

	// forward sample to video sink
	TSharedRef<FVlcMediaTextureSample, ESPMode::ThreadSafe> Sample = MakeShareable(
		new FVlcMediaTextureSample(
			MakeShareable(Picture, [](void* Ptr) { FMemory::Free(Ptr); }),
			Output->VideoBufferDim,
			Output->VideoOutputDim,
			Output->VideoSampleFormat,
			Output->VideoBufferStride,
			PinnedTimeInfo->Time,
			SampleTimecode,
			FTimespan::MaxValue()
		)
	);

	VideoSink->OnTextureSample(Sample);
}


void* FVlcMediaOutput::StaticVideoLockCallback(void* Opaque, void** Planes)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("StaticVideoLockCallback"));

	auto Output = (FVlcMediaOutput*)Opaque;

	if ((Output == nullptr) || (Output->VideoOutputDim.GetMin() <= 0))
	{
		return nullptr;
	}

	// allocate buffer for video frame
	FMemory::Memzero(Planes, FVlc::MaxPlanes * sizeof(void*));
	Planes[0] = FMemory::Malloc(Output->VideoBufferStride * Output->VideoBufferDim.Y, 32);

	return Planes[0];
}


unsigned FVlcMediaOutput::StaticVideoSetupCallback(void** Opaque, char* Chroma, unsigned* Width, unsigned* Height, unsigned* Pitches, unsigned* Lines)
{
	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("StaticVideoSetupCallback: Chroma=%s Dim=%ix%i"), ANSI_TO_TCHAR(Chroma), *Width, *Height);

	auto Output = *(FVlcMediaOutput**)Opaque;
	
	if (Output == nullptr)
	{
		return 0;
	}

	// get video output size
	if (FVlc::VideoGetSize(Output->Player, 0, (uint32*)&Output->VideoOutputDim.X, (uint32*)&Output->VideoOutputDim.Y) != 0)
	{
		Output->VideoBufferDim = FIntPoint::ZeroValue;
		Output->VideoOutputDim = FIntPoint::ZeroValue;
		Output->VideoBufferStride = 0;

		return 0;
	}

	if (Output->VideoOutputDim.GetMin() <= 0)
	{
		return 0;
	}

	// determine decoder & sample formats
	Output->VideoBufferDim = FIntPoint(*Width, *Height);

	if (FCStringAnsi::Stricmp(Chroma, "AYUV") == 0)
	{
		Output->VideoSampleFormat = EMediaTextureSampleFormat::CharAYUV;
		Output->VideoBufferStride = *Width * 4;
	}
	else if (FCStringAnsi::Stricmp(Chroma, "RV32") == 0)
	{
		Output->VideoSampleFormat = EMediaTextureSampleFormat::CharBGRA;
		Output->VideoBufferStride = *Width * 4;
	}
	else if ((FCStringAnsi::Stricmp(Chroma, "UYVY") == 0) ||
		(FCStringAnsi::Stricmp(Chroma, "Y422") == 0) ||
		(FCStringAnsi::Stricmp(Chroma, "UYNV") == 0) ||
		(FCStringAnsi::Stricmp(Chroma, "HDYC") == 0))
	{
		Output->VideoSampleFormat = EMediaTextureSampleFormat::CharUYVY;
		Output->VideoBufferStride = *Width * 2;
	}
	else if ((FCStringAnsi::Stricmp(Chroma, "YUY2") == 0) ||
		(FCStringAnsi::Stricmp(Chroma, "V422") == 0) ||
		(FCStringAnsi::Stricmp(Chroma, "YUYV") == 0))
	{
		Output->VideoSampleFormat = EMediaTextureSampleFormat::CharYUY2;
		Output->VideoBufferStride = *Width * 2;
	}
	else if (FCStringAnsi::Stricmp(Chroma, "YVYU") == 0)
	{
		Output->VideoSampleFormat = EMediaTextureSampleFormat::CharYVYU;
		Output->VideoBufferStride = *Width * 2;
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

			Output->VideoBufferDim = FIntPoint(Align(Output->VideoOutputDim.X, 16) / 2, Align(Output->VideoOutputDim.Y, 16));
			Output->VideoSampleFormat = EMediaTextureSampleFormat::CharYUY2;
			Output->VideoBufferStride = Output->VideoBufferDim.X * 4;
			*Height = Output->VideoBufferDim.Y;
		}
		else
		{
			FMemory::Memcpy(Chroma, "RV32", 4);

			Output->VideoBufferDim = Output->VideoOutputDim;
			Output->VideoSampleFormat = EMediaTextureSampleFormat::CharBGRA;
			Output->VideoBufferStride = Output->VideoBufferDim.X * 4;
		}
	}

	Lines[0] = Output->VideoBufferDim.Y;
	Pitches[0] = Output->VideoBufferStride;

	return 1;
}


void FVlcMediaOutput::StaticVideoUnlockCallback(void* Opaque, void* Picture, void* const* /*Planes*/)
{
	// do nothing
}
