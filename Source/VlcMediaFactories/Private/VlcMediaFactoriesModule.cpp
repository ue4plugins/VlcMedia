// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "VlcMediaFactoriesPCH.h"
#include "IMediaPlayerFactory.h"
#include "ModuleInterface.h"


#define LOCTEXT_NAMESPACE "FVlcMediaFactoriesModule"


/**
 * Implements the VlcMediaFactories module.
 */
class FVlcMediaFactoriesModule
	: public IMediaPlayerFactory
	, public IModuleInterface
{
public:

	//~ IMediaPlayerFactory interface

	virtual bool CanPlayUrl(const FString& Url, const IMediaOptions& Options, TArray<FText>* OutWarnings, TArray<FText>* OutErrors) const override
	{
		FString Scheme;
		FString Location;

		// check scheme
		if (!Url.Split(TEXT("://"), &Scheme, &Location, ESearchCase::CaseSensitive))
		{
			if (OutErrors != nullptr)
			{
				OutErrors->Add(LOCTEXT("NoSchemeFound", "No URI scheme found"));
			}

			return false;
		}

		if (!SupportedUriSchemes.Contains(Scheme))
		{
			if (OutErrors != nullptr)
			{
				OutErrors->Add(FText::Format(LOCTEXT("SchemeNotSupported", "The URI scheme '{0}' is not supported"), FText::FromString(Scheme)));
			}

			return false;
		}

		// check file extension
		if (Scheme == TEXT("file"))
		{
			const FString Extension = FPaths::GetExtension(Location, false);

			if (!SupportedFileExtensions.Contains(Extension))
			{
				if (OutErrors != nullptr)
				{
					OutErrors->Add(FText::Format(LOCTEXT("ExtensionNotSupported", "The file extension '{0}' is not supported"), FText::FromString(Extension)));
				}

				return false;
			}
		}

		// check options
		if (OutWarnings != nullptr)
		{
			if (Options.GetMediaOption("PrecacheFile", false) && (Scheme != TEXT("file")))
			{			
				OutWarnings->Add(LOCTEXT("PrecacheFileWarning", "Precaching is supported for local files only"));
			}
		}

		return true;
	}

	virtual TSharedPtr<IMediaPlayer> CreatePlayer() override
	{
		auto VlcMediaModule = FModuleManager::LoadModulePtr<IVlcMediaModule>("VlcMedia");
		return (VlcMediaModule != nullptr) ? VlcMediaModule->CreatePlayer() : nullptr;
	}

	virtual FText GetDisplayName() const override
	{
		return LOCTEXT("MediaPlayerDisplayName", "VideoLAN Media Player");
	}

	virtual FName GetName() const override
	{
		static FName PlayerName(TEXT("VlcMedia"));
		return PlayerName;
	}

	virtual const TArray<FString>& GetSupportedPlatforms() const override
	{
		return SupportedPlatforms;
	}

public:

	//~ IModuleInterface interface

	virtual void StartupModule() override
	{
		// supported file extensions
		SupportedFileExtensions.Add(TEXT("3gp"));
		SupportedFileExtensions.Add(TEXT("a52"));
		SupportedFileExtensions.Add(TEXT("aac"));
		SupportedFileExtensions.Add(TEXT("asf"));
		SupportedFileExtensions.Add(TEXT("au"));
		SupportedFileExtensions.Add(TEXT("avi"));
		SupportedFileExtensions.Add(TEXT("dts"));
		SupportedFileExtensions.Add(TEXT("dv"));
		SupportedFileExtensions.Add(TEXT("flac"));
		SupportedFileExtensions.Add(TEXT("flv"));
		SupportedFileExtensions.Add(TEXT("mkv"));
		SupportedFileExtensions.Add(TEXT("mka"));
		SupportedFileExtensions.Add(TEXT("mov"));
		SupportedFileExtensions.Add(TEXT("mp2"));
		SupportedFileExtensions.Add(TEXT("mp3"));
		SupportedFileExtensions.Add(TEXT("mp4"));
		SupportedFileExtensions.Add(TEXT("mpg"));
		SupportedFileExtensions.Add(TEXT("nsc"));
		SupportedFileExtensions.Add(TEXT("nsv"));
		SupportedFileExtensions.Add(TEXT("nut"));
		SupportedFileExtensions.Add(TEXT("ogm"));
		SupportedFileExtensions.Add(TEXT("ogg"));
		SupportedFileExtensions.Add(TEXT("ra"));
		SupportedFileExtensions.Add(TEXT("ram"));
		SupportedFileExtensions.Add(TEXT("rm"));
		SupportedFileExtensions.Add(TEXT("rmvb"));
		SupportedFileExtensions.Add(TEXT("rv"));
		SupportedFileExtensions.Add(TEXT("ts"));
		SupportedFileExtensions.Add(TEXT("tac"));
		SupportedFileExtensions.Add(TEXT("tta"));
		SupportedFileExtensions.Add(TEXT("ty"));
		SupportedFileExtensions.Add(TEXT("vid"));
		SupportedFileExtensions.Add(TEXT("wav"));
		SupportedFileExtensions.Add(TEXT("wmv"));
		SupportedFileExtensions.Add(TEXT("xa"));

		// supported platforms
		SupportedPlatforms.Add(TEXT("Linux"));
		SupportedPlatforms.Add(TEXT("Mac"));
		SupportedPlatforms.Add(TEXT("Win32"));
		SupportedPlatforms.Add(TEXT("Win64"));
		SupportedPlatforms.Add(TEXT("WinRT"));

		// supported schemes
		SupportedUriSchemes.Add(TEXT("cdda"));
		SupportedUriSchemes.Add(TEXT("file"));
		SupportedUriSchemes.Add(TEXT("dvd"));
		SupportedUriSchemes.Add(TEXT("ftp"));
		SupportedUriSchemes.Add(TEXT("http"));
		SupportedUriSchemes.Add(TEXT("https"));
		SupportedUriSchemes.Add(TEXT("mms"));
		SupportedUriSchemes.Add(TEXT("rtp"));
		SupportedUriSchemes.Add(TEXT("rtsp"));
		SupportedUriSchemes.Add(TEXT("sap"));
		SupportedUriSchemes.Add(TEXT("screen"));
		SupportedUriSchemes.Add(TEXT("vcd"));

		// register player factory
		auto MediaModule = FModuleManager::LoadModulePtr<IMediaModule>("Media");

		if (MediaModule != nullptr)
		{
			MediaModule->RegisterPlayerFactory(*this);
		}
	}

	virtual void ShutdownModule() override
	{
		// unregister player factory
		auto MediaModule = FModuleManager::GetModulePtr<IMediaModule>("Media");

		if (MediaModule != nullptr)
		{
			MediaModule->UnregisterPlayerFactory(*this);
		}
	}

private:

	/** List of supported media file types. */
	TArray<FString> SupportedFileExtensions;

	/** List of platforms that the media player support. */
	TArray<FString> SupportedPlatforms;

	/** List of supported URI schemes. */
	TArray<FString> SupportedUriSchemes;
};


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVlcMediaFactoriesModule, VlcMediaFactories);
