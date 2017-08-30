// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IMediaView.h"

struct FLibvlcMediaPlayer;
struct FLibvlcVideoViewpoint;


/**
 * Implements VLC video view settings.
 */
class FVlcMediaView
	: public IMediaView
{
public:

	/** Default constructor. */
	FVlcMediaView();

	/** Virtual destructor. */
	virtual ~FVlcMediaView();

public:

	/**
	 * Initialize this object for the specified VLC media player.
	 *
	 * @param InPlayer The VLC media player.
	 */
	void Initialize(FLibvlcMediaPlayer& InPlayer);

	/** Shut down this object. */
	void Shutdown();

public:

	//~ IMediaView interface

	virtual bool GetViewField(float& OutHorizontal, float& OutVertical) const override;
	virtual bool GetViewOrientation(FQuat& OutOrientation) const override;
	virtual bool SetViewField(float Horizontal, float Vertical, bool Absolute) override;
	virtual bool SetViewOrientation(const FQuat& Orientation, bool Absolute) override;

private:

	/** The current field of view (horizontal & vertical). */
	float CurrentFieldOfView;

	/** The current view orientation. */
	FQuat CurrentOrientation;

	/** The VLC media player object. */
	FLibvlcMediaPlayer* Player;

	/** The VLC video viewpoint. */
	FLibvlcVideoViewpoint* Viewpoint;
};
