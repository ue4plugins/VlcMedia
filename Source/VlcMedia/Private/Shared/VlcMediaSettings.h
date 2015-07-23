// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "VlcMediaSettings.generated.h"


UCLASS(config=Engine)
class UVlcMediaSettings
	: public UObject
{
	GENERATED_BODY()

public:

	/** Whether VLC log messages should be logged. */
	UPROPERTY(config, EditAnywhere, Category=Debugging)
	bool EnableLog;
};
