// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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
