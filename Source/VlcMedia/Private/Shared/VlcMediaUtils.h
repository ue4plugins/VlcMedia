// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


enum class ELibvlcState;
struct FLibvlcEvent;


namespace VlcMedia
{
	/**
	 * Convert a LibVLC event to string.
	 *
	 * @param Event The event code to convert.
	 * @return The corresponding string.
	 */
	FString EventToString(FLibvlcEvent* Event);

	/**
	 * Convert a LibVLC state to string.
	 *
	 * @param Event The state code to convert.
	 * @return The corresponding string.
	 */
	FString StateToString(ELibvlcState State);
}
