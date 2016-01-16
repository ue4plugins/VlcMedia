// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#pragma once


/* Dependencies
 *****************************************************************************/

#include "Core.h"
#include "CoreUObject.h"
#include "IMediaInfo.h"
#include "IMediaPlayer.h"
#include "IMediaSink.h"


/* Private macros
 *****************************************************************************/

DECLARE_LOG_CATEGORY_EXTERN(LogVlcMedia, Log, All);


/* Private includes
 *****************************************************************************/

#include "Vlc.h"
#include "VlcMediaTrack.h"
#include "VlcMediaAudioHandler.h"
#include "VlcMediaAudioTrack.h"
#include "VlcMediaCaptionTrack.h"
#include "VlcMediaSource.h"
#include "VlcMediaVideoHandler.h"
#include "VlcMediaVideoTrack.h"
#include "VlcMediaPlayer.h"
#include "VlcMediaSettings.h"
