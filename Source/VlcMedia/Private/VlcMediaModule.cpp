// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#include "IVlcMediaModule.h"
#include "VlcMediaPrivate.h"

#include "Vlc.h"
#include "VlcMediaPlayer.h"


DEFINE_LOG_CATEGORY(LogVlcMedia);

#define LOCTEXT_NAMESPACE "FVlcMediaModule"


/**
 * Implements the VlcMedia module.
 */
class FVlcMediaModule
	: public IVlcMediaModule
{
public:

	/** Default constructor. */
	FVlcMediaModule()
		: Initialized(false)
	{ }

public:

	//~ IVlcMediaModule interface

	virtual TSharedPtr<IMediaPlayer> CreatePlayer() override
	{
		if (!Initialized)
		{
			return nullptr;
		}

		return MakeShareable(new FVlcMediaPlayer(VlcInstance));
	}

	virtual void InitializeLogging() override
	{
		if (GetDefault<UVlcMediaSettings>()->EnableLog)
		{
			FVlc::LogSet(VlcInstance, &FVlcMediaModule::HandleVlcLog, nullptr);
		}
		else
		{
			FVlc::LogUnset(VlcInstance);
		}
	}

public:

	//~ IModuleInterface interface

	virtual void StartupModule() override
	{
		// initialize LibVLC
		if (!FVlc::Initialize())
		{
			UE_LOG(LogVlcMedia, Error, TEXT("Failed to initialize libvlc"));
			return;
		}

		// create LibVLC instance
		const ANSICHAR* Args[] =
		{
			TCHAR_TO_ANSI(*(FString(TEXT("--plugin-path=")) + FVlc::GetPluginDir())),
			"--aout", "amem",
			"--drop-late-frames",
			"--ignore-config",
			"--intf", "dummy",
			"--no-disable-screensaver",
			"--no-snapshot-preview",
#if (UE_BUILD_SHIPPING || UE_BUILD_TEST)
			"--no-stats",
#endif
			"--no-video-title-show",
#if PLATFORM_LINUX
			"--no-xlib",
#endif
			"--text-renderer", "dummy",
#if DEBUG
			"--verbose=2",
#else
			"--quiet",
#endif
			"--vout", "vmem",
		};

		int Argc = sizeof(Args) / sizeof(*Args);
		VlcInstance = FVlc::New(Argc, Args);

		if (VlcInstance == nullptr)
		{
			UE_LOG(LogVlcMedia, Warning, TEXT("Failed to create VLC instance (%s)"), ANSI_TO_TCHAR(FVlc::Errmsg()));
			FVlc::Shutdown();

			return;
		}

		// register logging callback
		InitializeLogging();	

		Initialized = true;
	}

	virtual void ShutdownModule() override
	{
		if (!Initialized)
		{
			return;
		}

		Initialized = false;

		// unregister logging callback
		FVlc::LogUnset(VlcInstance);

		// release LibVLC instance
		FVlc::Release((FLibvlcInstance*)VlcInstance);
		VlcInstance = nullptr;

		// shut down LibVLC
		FVlc::Shutdown();
	}

private:

	/** Handles log messages from LibVLC. */
	static void HandleVlcLog(void* /*Data*/, ELibvlcLogLevel Level, FLibvlcLog* Context, const char* Format, va_list Args)
	{
		ANSICHAR Message[1024];

		FCStringAnsi::GetVarArgs(Message, ARRAY_COUNT(Message), ARRAY_COUNT(Message) - 1, Format, Args);

		switch (Level)
		{
		case ELibvlcLogLevel::Debug:
			UE_LOG(LogVlcMedia, VeryVerbose, TEXT("%s"), ANSI_TO_TCHAR(Message));
			break;

		case ELibvlcLogLevel::Error:
			UE_LOG(LogVlcMedia, Error, TEXT("%s"), ANSI_TO_TCHAR(Message));
			break;

		case ELibvlcLogLevel::Notice:
			UE_LOG(LogVlcMedia, Verbose, TEXT("%s"), ANSI_TO_TCHAR(Message));
			break;

		case ELibvlcLogLevel::Warning:
			UE_LOG(LogVlcMedia, Warning, TEXT("%s"), ANSI_TO_TCHAR(Message));
			break;

		default:
			UE_LOG(LogVlcMedia, Log, TEXT("%s"), ANSI_TO_TCHAR(Message));
			break;
		}
	}

private:

	/** Whether the module has been initialized. */
	bool Initialized;

	/** The LibVLC instance. */
	FLibvlcInstance* VlcInstance;
};


IMPLEMENT_MODULE(FVlcMediaModule, VlcMedia);


#undef LOCTEXT_NAMESPACE
