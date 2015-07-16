// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#pragma once


/* Dependencies
 *****************************************************************************/

#include "Core.h"

#include "IMediaInfo.h"
#include "IMediaPlayer.h"
#include "IMediaSink.h"
#include "IMediaStream.h"
#include "IMediaAudioTrack.h"
#include "IMediaCaptionTrack.h"
#include "IMediaVideoTrack.h"


/* Private macros
 *****************************************************************************/

DECLARE_LOG_CATEGORY_EXTERN(LogVlcMedia, Log, All);


/* Private includes
 *****************************************************************************/

#include "Vlc.h"
#include "VlcMediaTrack.h"
#include "VlcMediaAudioTrack.h"
#include "VlcMediaCaptionTrack.h"
#include "VlcMediaVideoTrack.h"
#include "VlcMediaPlayer.h"
