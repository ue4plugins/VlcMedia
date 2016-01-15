// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "VlcMediaPrivatePCH.h"


/* FVlcMediaReader structors
*****************************************************************************/

FVlcMediaSource::FVlcMediaSource(FLibvlcInstance* InVlcInstance)
	: Media(nullptr)
	, VlcInstance(InVlcInstance)
{ }


/* FVlcMediaReader interface
*****************************************************************************/

FLibvlcMedia* FVlcMediaSource::OpenArchive(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl)
{
	Close();

	Data = Archive;
	Media = FVlc::MediaNewCallbacks(
		VlcInstance,
		nullptr,//&FVlcMediaReader::HandleMediaOpen,
		&FVlcMediaSource::HandleMediaRead,
		&FVlcMediaSource::HandleMediaSeek,
		&FVlcMediaSource::HandleMediaClose,
		this
	);

	if (Media == nullptr)
	{
		UE_LOG(LogVlcMedia, Warning, TEXT("Failed to open media from archive: %s"), ANSI_TO_TCHAR(FVlc::Errmsg()));
		Data.Reset();
	}

	CurrentUrl = OriginalUrl;

	return Media;
}


FLibvlcMedia* FVlcMediaSource::OpenUrl(const FString& Url)
{
	Close();

	Media = FVlc::MediaNewLocation(VlcInstance, TCHAR_TO_ANSI(*Url));

	if (Media == nullptr)
	{
		UE_LOG(LogVlcMedia, Warning, TEXT("Failed to open media %s: %s"), *Url, ANSI_TO_TCHAR(FVlc::Errmsg()));
	}

	CurrentUrl = Url;

	return Media;
}


void FVlcMediaSource::Close()
{
	if (Media != nullptr)
	{
		FVlc::MediaRelease(Media);
		Media = nullptr;
	}

	Data.Reset();
	CurrentUrl.Reset();
}


/* FVlcMediaReader static functions
*****************************************************************************/

int FVlcMediaSource::HandleMediaOpen(void* Opaque, void** OutData, uint64* OutSize)
{
	auto Reader = (FVlcMediaSource*)Opaque;

	if ((Reader == nullptr) || !Reader->Data.IsValid())
	{
		return 0;
	}

	*OutSize = Reader->Data->TotalSize();

	return 0;
}


SSIZE_T FVlcMediaSource::HandleMediaRead(void* Opaque, void* Buffer, SIZE_T Length)
{
	auto Reader = (FVlcMediaSource*)Opaque;

	if (Reader == nullptr)
	{
		return -1;
	}

	TSharedPtr<FArchive, ESPMode::ThreadSafe> Data = Reader->Data;

	if (!Reader->Data.IsValid())
	{
		return -1;
	}

	SIZE_T DataSize = (SIZE_T)Data->TotalSize();
	SIZE_T BytesToRead = FMath::Min(Length, DataSize);
	SIZE_T DataPosition = Reader->Data->Tell();

	if ((DataSize - BytesToRead) < DataPosition)
	{
		BytesToRead = DataSize - DataPosition;
	}

	if (BytesToRead > 0)
	{
		Data->Serialize(Buffer, BytesToRead);
	}

	return (SSIZE_T)BytesToRead;
}


int FVlcMediaSource::HandleMediaSeek(void* Opaque, uint64 Offset)
{
	auto Reader = (FVlcMediaSource*)Opaque;

	if (Reader == nullptr)
	{
		return -1;
	}

	TSharedPtr<FArchive, ESPMode::ThreadSafe> Data = Reader->Data;

	if (!Reader->Data.IsValid())
	{
		return -1;
	}

	if ((uint64)Data->TotalSize() <= Offset)
	{
		return -1;
	}

	Reader->Data->Seek(Offset);

	return 0;
}


void FVlcMediaSource::HandleMediaClose(void* Opaque)
{
	auto Reader = (FVlcMediaSource*)Opaque;

	if (Reader != nullptr)
	{
		Reader->Data->Seek(0);
	}
}
