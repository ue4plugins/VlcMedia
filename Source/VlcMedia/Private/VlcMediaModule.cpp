// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "IVlcMediaModule.h"
#include "VlcMediaPrivate.h"

#include "HAL/FileManager.h"
#include "Misc/OutputDeviceFile.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"

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

	virtual TSharedPtr<IMediaPlayer, ESPMode::ThreadSafe> CreatePlayer(IMediaEventSink& EventSink) override
	{
		if (!Initialized)
		{
			return nullptr;
		}

		return MakeShared<FVlcMediaPlayer, ESPMode::ThreadSafe>(EventSink, VlcInstance);
	}

public:

	//~ IModuleInterface interface

	virtual void StartupModule() override
	{
		// initialize LibVLC
		if (!FVlc::Initialize())
		{
			UE_LOG(LogVlcMedia, Error, TEXT("Failed to initialize LibVLC"));
			return;
		}

		UE_LOG(LogVlcMedia, Log, TEXT("Initialized LibVLC %s (%s - %s)"),
			ANSI_TO_TCHAR(FVlc::GetVersion()),
			ANSI_TO_TCHAR(FVlc::GetChangeset()),
			ANSI_TO_TCHAR(FVlc::GetCompiler())
		);

#if UE_BUILD_DEBUG
		// backup old log file
		const FString LogFilePath = FPaths::Combine(FPaths::ProjectLogDir(), TEXT("vlc.log"));
		FOutputDeviceFile::CreateBackupCopy(*LogFilePath);
		IFileManager::Get().Delete(*LogFilePath);
#endif

		const auto Settings = GetDefault<UVlcMediaSettings>();

		// create LibVLC instance
		const ANSICHAR* Args[] =
		{
			// caching
			TCHAR_TO_ANSI(*(FString::Printf(TEXT("--disc-caching=%i"), (int32)Settings->DiscCaching.GetTotalMilliseconds()))),
			TCHAR_TO_ANSI(*(FString::Printf(TEXT("--file-caching=%i"), (int32)Settings->FileCaching.GetTotalMilliseconds()))),
			TCHAR_TO_ANSI(*(FString::Printf(TEXT("--live-caching=%i"), (int32)Settings->LiveCaching.GetTotalMilliseconds()))),
			TCHAR_TO_ANSI(*(FString::Printf(TEXT("--network-caching=%i"), (int32)Settings->NetworkCaching.GetTotalMilliseconds()))),

			// config
			"--ignore-config",

			// logging
#if UE_BUILD_DEBUG
			"--file-logging",
			TCHAR_TO_ANSI(*(FString(TEXT("--logfile=")) + LogFilePath)),
#endif

#if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT)
			"--verbose=2",
#else
			"--quiet",
#endif

			// output
			"--aout", "amem",
			"--intf", "dummy",
			"--text-renderer", "dummy",
			"--vout", "vmem",

			// performance
			"--drop-late-frames",

			// undesired features
			"--no-disable-screensaver",
			"--no-plugins-cache",
			"--no-snapshot-preview",
			"--no-video-title-show",

#if (UE_BUILD_SHIPPING || UE_BUILD_TEST)
			"--no-stats",
#endif

#if PLATFORM_LINUX
			"--no-xlib",
#endif
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
		FVlc::LogSet(VlcInstance, &FVlcMediaModule::HandleVlcLog, nullptr);

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
#if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT)
		const auto Settings = GetDefault<UVlcMediaSettings>();

		// filter unwanted messages
		if ((uint8)Level < (uint8)Settings->LogLevel)
		{
			return;
		}

		FString LogContext;

		// get context information
		if (Context != nullptr)
		{
			const char* Module = nullptr;
			const char* File = nullptr;
			unsigned Line = 0;

			FVlc::LogGetContext(Context, &Module, &File, &Line);
			LogContext = FString::Printf(TEXT("%s: "), (Module != nullptr) ? ANSI_TO_TCHAR(Module) : TEXT("unknown module"));

			if (Settings->ShowLogContext)
			{
				LogContext += FString::Printf(TEXT("%s, line %s: "),
					(File != nullptr) ? ANSI_TO_TCHAR(File) : TEXT("unknown file"),
					(Line != 0) ? *FString::Printf(TEXT("%i"), Line) : TEXT("n/a")
				);
			}
		}
		else
		{
			LogContext = TEXT("generic: ");
		}

		// forward message to log
		ANSICHAR Message[1024];

		FCStringAnsi::GetVarArgs(Message, UE_ARRAY_COUNT(Message), Format, Args);

		switch (Level)
		{
		case ELibvlcLogLevel::Debug:
			UE_LOG(LogVlcMedia, VeryVerbose, TEXT("%s%s"), *LogContext, ANSI_TO_TCHAR(Message));
			break;

		case ELibvlcLogLevel::Error:
			UE_LOG(LogVlcMedia, Error, TEXT("%s%s"), *LogContext, ANSI_TO_TCHAR(Message));
			break;

		case ELibvlcLogLevel::Notice:
			UE_LOG(LogVlcMedia, Verbose, TEXT("%s%s"), *LogContext, ANSI_TO_TCHAR(Message));
			break;

		case ELibvlcLogLevel::Warning:
			UE_LOG(LogVlcMedia, Warning, TEXT("%s%s"), *LogContext, ANSI_TO_TCHAR(Message));
			break;

		default:
			UE_LOG(LogVlcMedia, Log, TEXT("%s%s"), *LogContext, ANSI_TO_TCHAR(Message));
			break;
		}
#endif
	}

private:

	/** Whether the module has been initialized. */
	bool Initialized;

	/** The LibVLC instance. */
	FLibvlcInstance* VlcInstance;
};


IMPLEMENT_MODULE(FVlcMediaModule, VlcMedia);


#undef LOCTEXT_NAMESPACE
