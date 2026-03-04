// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "IVlcMediaModule.h"
#include "VlcMediaPrivate.h"

#include "HAL/FileManager.h"
#include "Containers/StringConv.h"
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
		UE_LOG(LogVlcMedia, Warning, TEXT("FVlcMediaModule::CreatePlayer called (Initialized=%s)"), Initialized ? TEXT("true") : TEXT("false"));
		if (!Initialized)
		{
			InitializeLibVlc();
			if (!Initialized)
			{
				return nullptr;
			}
		}

		return MakeShared<FVlcMediaPlayer, ESPMode::ThreadSafe>(EventSink, VlcInstance);
	}

public:

	//~ IModuleInterface interface

	virtual void StartupModule() override
	{
		UE_LOG(LogVlcMedia, Warning, TEXT("FVlcMediaModule::StartupModule"));
		InitializeLibVlc();
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
	bool InitializeLibVlc()
	{
		if (Initialized)
		{
			return true;
		}

		// initialize LibVLC
		if (!FVlc::Initialize())
		{
			UE_LOG(LogVlcMedia, Error, TEXT("Failed to initialize LibVLC"));
			return false;
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
		const FString PluginDir = FVlc::GetPluginDir();
		const FString PluginPathArg = FString(TEXT("--plugin-path=")) + PluginDir;
		FPlatformMisc::SetEnvironmentVar(TEXT("VLC_PLUGIN_PATH"), *PluginDir);
		UE_LOG(LogVlcMedia, Warning, TEXT("VLC PluginDir: %s"), *PluginDir);

		TArray<FString> ArgStrings;
		ArgStrings.Reserve(32);
		ArgStrings.Add(FString::Printf(TEXT("--disc-caching=%i"), (int32)Settings->DiscCaching.GetTotalMilliseconds()));
		ArgStrings.Add(FString::Printf(TEXT("--file-caching=%i"), (int32)Settings->FileCaching.GetTotalMilliseconds()));
		ArgStrings.Add(FString::Printf(TEXT("--live-caching=%i"), (int32)Settings->LiveCaching.GetTotalMilliseconds()));
		ArgStrings.Add(FString::Printf(TEXT("--network-caching=%i"), (int32)Settings->NetworkCaching.GetTotalMilliseconds()));

		ArgStrings.Add(TEXT("--ignore-config"));

#if UE_BUILD_DEBUG
		ArgStrings.Add(TEXT("--file-logging"));
		ArgStrings.Add(FString(TEXT("--logfile=")) + LogFilePath);
#endif

#if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT)
		ArgStrings.Add(TEXT("--verbose=2"));
#else
		ArgStrings.Add(TEXT("--quiet"));
#endif

		ArgStrings.Add(TEXT("--aout=amem"));
		ArgStrings.Add(TEXT("--intf=dummy"));
		ArgStrings.Add(TEXT("--text-renderer=dummy"));
		ArgStrings.Add(TEXT("--vout=vmem"));
		ArgStrings.Add(TEXT("--rtsp-tcp"));

		ArgStrings.Add(TEXT("--drop-late-frames"));
		ArgStrings.Add(TEXT("--avcodec-hw=none"));

		ArgStrings.Add(TEXT("--no-disable-screensaver"));
		ArgStrings.Add(TEXT("--no-plugins-cache"));
		ArgStrings.Add(TEXT("--no-snapshot-preview"));
		ArgStrings.Add(TEXT("--no-video-title-show"));

#if (UE_BUILD_SHIPPING || UE_BUILD_TEST)
		ArgStrings.Add(TEXT("--no-stats"));
#endif

#if PLATFORM_LINUX
		ArgStrings.Add(TEXT("--no-xlib"));
#endif

		TArray<FTCHARToUTF8> ArgUtf8;
		ArgUtf8.Reserve(ArgStrings.Num());

		TArray<const ANSICHAR*> Argv;
		Argv.Reserve(ArgStrings.Num());

		for (const FString& Arg : ArgStrings)
		{
			ArgUtf8.Emplace(*Arg);
			Argv.Add(ArgUtf8.Last().Get());
		}

		VlcInstance = FVlc::New(Argv.Num(), Argv.GetData());

		if (VlcInstance == nullptr)
		{
			UE_LOG(LogVlcMedia, Warning, TEXT("libvlc_new failed; retrying with empty args"));
			VlcInstance = FVlc::New(0, nullptr);
		}

		if (VlcInstance == nullptr)
		{
			UE_LOG(LogVlcMedia, Warning, TEXT("Failed to create VLC instance (%s)"), ANSI_TO_TCHAR(FVlc::Errmsg()));
			FVlc::Shutdown();

			return false;
		}

		// register logging callback
		FVlc::LogSet(VlcInstance, &FVlcMediaModule::HandleVlcLog, nullptr);

		Initialized = true;
		return true;
	}


	/** Handles log messages from LibVLC. */
	static void HandleVlcLog(void* /*Data*/, ELibvlcLogLevel Level, FLibvlcLog* Context, const char* Format, va_list Args)
	{
		// [DEBUG] Force enable logging for all builds and disable filtering
		// #if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT)
		
		// const auto Settings = GetDefault<UVlcMediaSettings>();

		// [DEBUG] Disable filtering
		// if ((uint8)Level < (uint8)Settings->LogLevel)
		// {
		// 	return;
		// }

		FString LogContext;

		// get context information
		if (Context != nullptr)
		{
			const char* Module = nullptr;
			const char* File = nullptr;
			unsigned Line = 0;

			FVlc::LogGetContext(Context, &Module, &File, &Line);
			LogContext = FString::Printf(TEXT("%s: "), (Module != nullptr) ? ANSI_TO_TCHAR(Module) : TEXT("unknown module"));

			// [DEBUG] Always show context
			// if (Settings->ShowLogContext)
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

		// [DEBUG] Force all logs to Warning so they show up in UE Editor Log
		UE_LOG(LogVlcMedia, Warning, TEXT("[VLC Internal] %s%s"), *LogContext, ANSI_TO_TCHAR(Message));

		// #endif
	}

private:

	/** Whether the module has been initialized. */
	bool Initialized;

	/** The LibVLC instance. */
	FLibvlcInstance* VlcInstance;
};


IMPLEMENT_MODULE(FVlcMediaModule, VlcMedia);


#undef LOCTEXT_NAMESPACE
