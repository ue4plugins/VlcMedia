// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "VlcMediaSettings.generated.h"


/**
 * Available levels for LibVLC log messages.
 */
UENUM()
enum class EVlcMediaLogLevel : uint8
{
	/** Error messages. */
	Error = 0,

	/** Warnings and potential errors. */
	Warning = 1,

	/** Debug messages. */
	Debug = 2,
};


/**
 * Settings for the VlcMedia plug-in.
 */
UCLASS(config=Engine)
class VLCMEDIAFACTORY_API UVlcMediaSettings
	: public UObject
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UVlcMediaSettings();

public:

	/** Caching duration for optical media (default = 300 ms). */
	UPROPERTY(config, EditAnywhere, Category=Caching)
	FTimespan DiscCaching;

	/** Caching duration for local files (default = 300 ms). */
	UPROPERTY(config, EditAnywhere, Category=Caching)
	FTimespan FileCaching;

	/** Caching duration for cameras and microphones (default = 300 ms). */
	UPROPERTY(config, EditAnywhere, Category=Caching)
	FTimespan LiveCaching;

	/** Caching duration for network resources (default = 1000 ms). */
	UPROPERTY(config, EditAnywhere, Category=Caching)
	FTimespan NetworkCaching;

public:

	/**
	 * Log level for LibVLC log messages to be forwarded to UE4 log file (default = Warning).
	 *
	 * This setting is used only in Debug and Development builds.
	 * No log entries will written in Shipping and Test builds.
	 */
	UPROPERTY(config, EditAnywhere, Category=Debugging)
	EVlcMediaLogLevel LogLevel;

	/** Whether to include file name & line number in LibVLC log messages. */
	UPROPERTY(config, EditAnywhere, Category=Debugging)
	bool ShowLogContext;
};
