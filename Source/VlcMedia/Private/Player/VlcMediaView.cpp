// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VlcMediaView.h"
#include "VlcMediaPrivate.h"

#include "Vlc.h"


/* FVlcMediaView structors
*****************************************************************************/

FVlcMediaView::FVlcMediaView()
	: CurrentFieldOfView(90.0f)
	, CurrentOrientation(EForceInit::ForceInit)
	, Player(nullptr)
{
	Viewpoint = FVlc::VideoNewViewpoint();

	if (Viewpoint == nullptr)
	{
		UE_LOG(LogVlcMedia, Warning, TEXT("Failed to create viewpoint structure; 360 view controls will be unavailable."));
	}
}


FVlcMediaView::~FVlcMediaView()
{
	if (Viewpoint != nullptr)
	{
		FVlc::Free(Viewpoint);
		Viewpoint = nullptr;
	}
}


/* FVlcMediaView interface
*****************************************************************************/

void FVlcMediaView::Initialize(FLibvlcMediaPlayer& InPlayer)
{
	Player = &InPlayer;
}


void FVlcMediaView::Shutdown()
{
	Player = nullptr;
}


/* IMediaView interface
*****************************************************************************/

bool FVlcMediaView::GetViewField(float& OutHorizontal, float& OutVertical) const
{
	OutHorizontal = CurrentFieldOfView;
	OutVertical = CurrentFieldOfView;

	return true;
}


bool FVlcMediaView::GetViewOrientation(FQuat& OutOrientation) const
{
	OutOrientation = CurrentOrientation;

	return true;
}


bool FVlcMediaView::SetViewField(float Horizontal, float Vertical, bool Absolute)
{
	if ((Player == nullptr) || (Viewpoint == nullptr))
	{
		return false;
	}

	if (!Absolute)
	{
		Horizontal = CurrentFieldOfView + Horizontal;
	}

	FVector Euler = CurrentOrientation.Euler();

	Viewpoint->Roll = Euler.X;
	Viewpoint->Pitch = Euler.Y;
	Viewpoint->Yaw = Euler.Z;
	Viewpoint->FieldOfView = FMath::ClampAngle(Horizontal, 10.0f, 360.0f);

	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("Setting viewpoint to %f %f %f / %f."), Viewpoint->Roll, Viewpoint->Pitch, Viewpoint->Roll, Viewpoint->FieldOfView);

	if (FVlc::VideoUpdateViewpoint(Player, Viewpoint, true) != 0)
	{
		return false;
	}

	CurrentFieldOfView = Horizontal;

	return true;
}


bool FVlcMediaView::SetViewOrientation(const FQuat& Orientation, bool Absolute)
{
	if ((Player == nullptr) || (Viewpoint == nullptr))
	{
		return false;
	}

	FQuat NewOrientation;

	if (Absolute)
	{
		NewOrientation = Orientation;
	}
	else
	{
		NewOrientation = Orientation * CurrentOrientation;
	}

	FVector Euler = NewOrientation.Euler();

	Viewpoint->Roll = Euler.X;
	Viewpoint->Pitch = Euler.Y;
	Viewpoint->Yaw = Euler.Z;
	Viewpoint->FieldOfView = CurrentFieldOfView;

	UE_LOG(LogVlcMedia, VeryVerbose, TEXT("Setting viewpoint to %f %f %f / %f."), Viewpoint->Roll, Viewpoint->Pitch, Viewpoint->Roll, Viewpoint->FieldOfView);

	if (FVlc::VideoUpdateViewpoint(Player, Viewpoint, true) != 0)
	{
		return false;
	}

	CurrentOrientation = NewOrientation;

	return true;
}
