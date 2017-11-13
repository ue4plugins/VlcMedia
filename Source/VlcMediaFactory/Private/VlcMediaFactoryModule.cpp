// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "IMediaPlayerFactory.h"
#include "IMediaModule.h"
#include "IMediaOptions.h"
#include "Internationalization/Internationalization.h"
#include "Misc/Paths.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "UObject/Class.h"
#include "UObject/NameTypes.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"

#if WITH_EDITOR
	#include "ISettingsModule.h"
	#include "ISettingsSection.h"
	#include "VlcMediaSettings.h"
#endif

#include "../../VlcMedia/Public/IVlcMediaModule.h"


#define LOCTEXT_NAMESPACE "FVlcMediaFactoryModule"


/**
 * Implements the VlcMediaFactory module.
 */
class FVlcMediaFactoryModule
	: public IMediaPlayerFactory
	, public IModuleInterface
{
public:

	//~ IMediaPlayerFactory interface

	virtual bool CanPlayUrl(const FString& Url, const IMediaOptions* Options, TArray<FText>* OutWarnings, TArray<FText>* OutErrors) const override
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
		if ((OutWarnings != nullptr) && (Options != nullptr))
		{
			if (Options->GetMediaOption("PrecacheFile", false) && (Scheme != TEXT("file")))
			{
				OutWarnings->Add(LOCTEXT("PrecacheFileWarning", "Precaching is supported for local files only"));
			}
		}

		return true;
	}

	virtual TSharedPtr<IMediaPlayer, ESPMode::ThreadSafe> CreatePlayer(IMediaEventSink& EventSink) override
	{
		auto VlcMediaModule = FModuleManager::LoadModulePtr<IVlcMediaModule>("VlcMedia");
		return (VlcMediaModule != nullptr) ? VlcMediaModule->CreatePlayer(EventSink) : nullptr;
	}

	virtual FText GetDisplayName() const override
	{
		return LOCTEXT("MediaPlayerDisplayName", "VideoLAN Player");
	}

	virtual FName GetPlayerName() const override
	{
		static FName PlayerName(TEXT("VlcMedia"));
		return PlayerName;
	}

	virtual const TArray<FString>& GetSupportedPlatforms() const override
	{
		return SupportedPlatforms;
	}

	virtual bool SupportsFeature(EMediaFeature Feature) const override
	{
		return ((Feature == EMediaFeature::AudioSamples) ||
				(Feature == EMediaFeature::AudioTracks) ||
				(Feature == EMediaFeature::CaptionTracks) ||
				(Feature == EMediaFeature::Video360) ||
				(Feature == EMediaFeature::VideoSamples) ||
				(Feature == EMediaFeature::VideoTracks));

	}

public:

	//~ IModuleInterface interface

	virtual void StartupModule() override
	{
		// supported file extensions
		SupportedFileExtensions.Add(TEXT("3g2"));
		SupportedFileExtensions.Add(TEXT("3ga"));
		SupportedFileExtensions.Add(TEXT("3gp"));
		SupportedFileExtensions.Add(TEXT("3gp2"));
		SupportedFileExtensions.Add(TEXT("3gpp"));
		SupportedFileExtensions.Add(TEXT("669"));
		SupportedFileExtensions.Add(TEXT("a52"));
		SupportedFileExtensions.Add(TEXT("aac"));
		SupportedFileExtensions.Add(TEXT("ac3"));
		SupportedFileExtensions.Add(TEXT("adt"));
		SupportedFileExtensions.Add(TEXT("adts"));
		SupportedFileExtensions.Add(TEXT("aif"));
		SupportedFileExtensions.Add(TEXT("aifc"));
		SupportedFileExtensions.Add(TEXT("aiff"));
		SupportedFileExtensions.Add(TEXT("amb"));
		SupportedFileExtensions.Add(TEXT("amr"));
		SupportedFileExtensions.Add(TEXT("amv"));
		SupportedFileExtensions.Add(TEXT("aob"));
		SupportedFileExtensions.Add(TEXT("ape"));
		SupportedFileExtensions.Add(TEXT("asf"));
		SupportedFileExtensions.Add(TEXT("au"));
		SupportedFileExtensions.Add(TEXT("avi"));
		SupportedFileExtensions.Add(TEXT("awb"));
		SupportedFileExtensions.Add(TEXT("bik"));
		SupportedFileExtensions.Add(TEXT("bin"));
		SupportedFileExtensions.Add(TEXT("caf"));
		SupportedFileExtensions.Add(TEXT("crf"));
		SupportedFileExtensions.Add(TEXT("divx"));
		SupportedFileExtensions.Add(TEXT("drc"));
		SupportedFileExtensions.Add(TEXT("dts"));
		SupportedFileExtensions.Add(TEXT("dv"));
		SupportedFileExtensions.Add(TEXT("evo"));
		SupportedFileExtensions.Add(TEXT("f4v"));
		SupportedFileExtensions.Add(TEXT("flac"));
		SupportedFileExtensions.Add(TEXT("flv"));
		SupportedFileExtensions.Add(TEXT("gvi"));
		SupportedFileExtensions.Add(TEXT("gxf"));
		SupportedFileExtensions.Add(TEXT("iso"));
		SupportedFileExtensions.Add(TEXT("it"));
		SupportedFileExtensions.Add(TEXT("kar"));
		SupportedFileExtensions.Add(TEXT("m1v"));
		SupportedFileExtensions.Add(TEXT("m2t"));
		SupportedFileExtensions.Add(TEXT("m2ts"));
		SupportedFileExtensions.Add(TEXT("m2v"));
		SupportedFileExtensions.Add(TEXT("m4a"));
		SupportedFileExtensions.Add(TEXT("m4b"));
		SupportedFileExtensions.Add(TEXT("m4p"));
		SupportedFileExtensions.Add(TEXT("m4v"));
		SupportedFileExtensions.Add(TEXT("m5p"));
		SupportedFileExtensions.Add(TEXT("mid"));
		SupportedFileExtensions.Add(TEXT("mka"));
		SupportedFileExtensions.Add(TEXT("mkv"));
		SupportedFileExtensions.Add(TEXT("mlp"));
		SupportedFileExtensions.Add(TEXT("mod"));
		SupportedFileExtensions.Add(TEXT("mov"));
		SupportedFileExtensions.Add(TEXT("mp1"));
		SupportedFileExtensions.Add(TEXT("mp2"));
		SupportedFileExtensions.Add(TEXT("mp2v"));
		SupportedFileExtensions.Add(TEXT("mp3"));
		SupportedFileExtensions.Add(TEXT("mp4"));
		SupportedFileExtensions.Add(TEXT("mp4v"));
		SupportedFileExtensions.Add(TEXT("mpa"));
		SupportedFileExtensions.Add(TEXT("mpc"));
		SupportedFileExtensions.Add(TEXT("mpe"));
		SupportedFileExtensions.Add(TEXT("mpeg"));
		SupportedFileExtensions.Add(TEXT("mpeg1"));
		SupportedFileExtensions.Add(TEXT("mpeg2"));
		SupportedFileExtensions.Add(TEXT("mpeg4"));
		SupportedFileExtensions.Add(TEXT("mpg"));
		SupportedFileExtensions.Add(TEXT("mpga"));
		SupportedFileExtensions.Add(TEXT("mpv2"));
		SupportedFileExtensions.Add(TEXT("mts"));
		SupportedFileExtensions.Add(TEXT("mtv"));
		SupportedFileExtensions.Add(TEXT("mus"));
		SupportedFileExtensions.Add(TEXT("mxf"));
		SupportedFileExtensions.Add(TEXT("mxg"));
		SupportedFileExtensions.Add(TEXT("nsc"));
		SupportedFileExtensions.Add(TEXT("nsv"));
		SupportedFileExtensions.Add(TEXT("nut"));
		SupportedFileExtensions.Add(TEXT("nuv"));
		SupportedFileExtensions.Add(TEXT("oga"));
		SupportedFileExtensions.Add(TEXT("ogg"));
		SupportedFileExtensions.Add(TEXT("ogm"));
		SupportedFileExtensions.Add(TEXT("ogv"));
		SupportedFileExtensions.Add(TEXT("ogx"));
		SupportedFileExtensions.Add(TEXT("oma"));
		SupportedFileExtensions.Add(TEXT("opus"));
		SupportedFileExtensions.Add(TEXT("qcp"));
		SupportedFileExtensions.Add(TEXT("ps"));
		SupportedFileExtensions.Add(TEXT("ra"));
		SupportedFileExtensions.Add(TEXT("ram"));
		SupportedFileExtensions.Add(TEXT("rec"));
		SupportedFileExtensions.Add(TEXT("rm"));
		SupportedFileExtensions.Add(TEXT("rmi"));
		SupportedFileExtensions.Add(TEXT("rmvb"));
		SupportedFileExtensions.Add(TEXT("rpl"));
		SupportedFileExtensions.Add(TEXT("rv"));
		SupportedFileExtensions.Add(TEXT("s3m"));
		SupportedFileExtensions.Add(TEXT("sdp"));
		SupportedFileExtensions.Add(TEXT("sid"));
		SupportedFileExtensions.Add(TEXT("spx"));
		SupportedFileExtensions.Add(TEXT("ts"));
		SupportedFileExtensions.Add(TEXT("tak"));
		SupportedFileExtensions.Add(TEXT("thd"));
		SupportedFileExtensions.Add(TEXT("thp"));
		SupportedFileExtensions.Add(TEXT("tod"));
		SupportedFileExtensions.Add(TEXT("tta"));
		SupportedFileExtensions.Add(TEXT("rp"));
		SupportedFileExtensions.Add(TEXT("rs"));
		SupportedFileExtensions.Add(TEXT("tta"));
		SupportedFileExtensions.Add(TEXT("tts"));
		SupportedFileExtensions.Add(TEXT("txd"));
		SupportedFileExtensions.Add(TEXT("ty"));
		SupportedFileExtensions.Add(TEXT("vid"));
		SupportedFileExtensions.Add(TEXT("vob"));
		SupportedFileExtensions.Add(TEXT("voc"));
		SupportedFileExtensions.Add(TEXT("vqf"));
		SupportedFileExtensions.Add(TEXT("vro"));
		SupportedFileExtensions.Add(TEXT("w64"));
		SupportedFileExtensions.Add(TEXT("wav"));
		SupportedFileExtensions.Add(TEXT("webm"));
		SupportedFileExtensions.Add(TEXT("wm"));
		SupportedFileExtensions.Add(TEXT("wma"));
		SupportedFileExtensions.Add(TEXT("wmv"));
		SupportedFileExtensions.Add(TEXT("wtv"));
		SupportedFileExtensions.Add(TEXT("wv"));
		SupportedFileExtensions.Add(TEXT("xa"));
		SupportedFileExtensions.Add(TEXT("xesc"));
		SupportedFileExtensions.Add(TEXT("xm"));

		// supported platforms
		SupportedPlatforms.Add(TEXT("Linux"));
		SupportedPlatforms.Add(TEXT("Mac"));
		SupportedPlatforms.Add(TEXT("Windows"));
		SupportedPlatforms.Add(TEXT("WinRT"));

		// supported schemes
		SupportedUriSchemes.Add(TEXT("cdda"));
		SupportedUriSchemes.Add(TEXT("dccp"));
		SupportedUriSchemes.Add(TEXT("dshow"));
		SupportedUriSchemes.Add(TEXT("dvd"));
		SupportedUriSchemes.Add(TEXT("file"));
		SupportedUriSchemes.Add(TEXT("dvd"));
		SupportedUriSchemes.Add(TEXT("ftp"));
		SupportedUriSchemes.Add(TEXT("http"));
		SupportedUriSchemes.Add(TEXT("https"));
		SupportedUriSchemes.Add(TEXT("icyx"));
		SupportedUriSchemes.Add(TEXT("itpc"));
		SupportedUriSchemes.Add(TEXT("mms"));
		SupportedUriSchemes.Add(TEXT("mmsh"));
		SupportedUriSchemes.Add(TEXT("mmst"));
		SupportedUriSchemes.Add(TEXT("mmsu"));
		SupportedUriSchemes.Add(TEXT("mtp"));
		SupportedUriSchemes.Add(TEXT("pnm"));
		SupportedUriSchemes.Add(TEXT("realrtsp"));
		SupportedUriSchemes.Add(TEXT("rtmp"));
		SupportedUriSchemes.Add(TEXT("rtp"));
		SupportedUriSchemes.Add(TEXT("rtsp"));
		SupportedUriSchemes.Add(TEXT("sap"));
		SupportedUriSchemes.Add(TEXT("smb"));
		SupportedUriSchemes.Add(TEXT("screen"));
		SupportedUriSchemes.Add(TEXT("unsv"));
		SupportedUriSchemes.Add(TEXT("v4l2"));
		SupportedUriSchemes.Add(TEXT("vcd"));

#if WITH_EDITOR
		// register settings
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "VlcMedia",
				LOCTEXT("VlcMediaSettingsName", "VLC Media"),
				LOCTEXT("VlcMediaSettingsDescription", "Configure the VLC Media plug-in."),
				GetMutableDefault<UVlcMediaSettings>()
			);
		}
#endif //WITH_EDITOR

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

#if WITH_EDITOR
		// unregister settings
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			SettingsModule->UnregisterSettings("Project", "Plugins", "VlcMedia");
		}
#endif //WITH_EDITOR
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

IMPLEMENT_MODULE(FVlcMediaFactoryModule, VlcMediaFactory);
