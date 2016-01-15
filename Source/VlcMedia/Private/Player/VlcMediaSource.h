// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#pragma once


struct FLibvlcInstance;
struct FLibvlcMedia;


class FVlcMediaSource
{
public:

	/**
	 * Create and initialize a new instance.
	 *
	 * @param InInstance The LibVLC instance to use.
	 */
	FVlcMediaSource(FLibvlcInstance* InVlcInstance);

public:

	/** Get the media object. */
	FLibvlcMedia* GetMedia() const
	{
		return Media;
	}

	/** Get the URL of the currently open media source. */
	const FString& GetCurrentUrl() const
	{
		return CurrentUrl;
	}

	/**
	 * Open a media source using the given archive.
	 *
	 * @param Archive The archive to read media data from.
	 * @return The media object.
	 * @see OpenUrl, Close
	 */
	FLibvlcMedia* OpenArchive(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl);

	/**
	 * Open a media source from the specified URL.
	 *
	 * @param Url The media resource locator.
	 * @return The media object.
	 * @see OpenArchive, Close
	 */
	FLibvlcMedia* OpenUrl(const FString& Url);

	/**
	 * Close the media source.
	 *
	 * @see OpenArchive, OpenUrl
	 */
	void Close();

private:

	/** Handles open callbacks from VLC. */
	static int HandleMediaOpen(void* Opaque, void** OutData, uint64* OutSize);

	/** Handles read callbacks from VLC. */
	static SSIZE_T HandleMediaRead(void* Opaque, void* Buffer, SIZE_T Length);

	/** Handles seek callbacks from VLC. */
	static int HandleMediaSeek(void* Opaque, uint64 Offset);

	/** Handles close callbacks from VLC. */
	static void HandleMediaClose(void* Opaque);

private:

	/** The file or memory archive to stream from (for local media only). */
	TSharedPtr<FArchive, ESPMode::ThreadSafe> Data;

	/** The media object. */
	FLibvlcMedia* Media;

	/** Currently opened media. */
	FString CurrentUrl;

	/** The LibVLC instance. */
	FLibvlcInstance* VlcInstance;
};
