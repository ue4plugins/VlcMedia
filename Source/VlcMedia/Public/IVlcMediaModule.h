// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"
#include "Templates/SharedPointer.h"

class IMediaEventSink;
class IMediaPlayer;


/**
 * Interface for the VlcMedia module.
 */
class IVlcMediaModule
	: public IModuleInterface
{
public:

	/**
	 * Create a VideoLAN based media player.
	 *
	 * @param EventSink The object that receives media events from the player.
	 * @return A new media player, or nullptr if a player couldn't be created.
	 */
	virtual TSharedPtr<IMediaPlayer, ESPMode::ThreadSafe> CreatePlayer(IMediaEventSink& EventSink) = 0;

public:

	/** Virtual destructor. */
	virtual ~IVlcMediaModule() { }
};
