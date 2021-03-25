// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VlcMediaUtils.h"
#include "VlcTypes.h"


namespace VlcMedia
{
	FString EventToString(FLibvlcEvent* Event)
	{
		switch (Event->Type)
		{
		case ELibvlcEventType::MediaMetaChanged: return TEXT("Media Data Changed");
		case ELibvlcEventType::MediaSubItemAdded: return TEXT("Media Sub Item Added");
		case ELibvlcEventType::MediaDurationChanged: return TEXT("Media Duration Changed");
		case ELibvlcEventType::MediaParsedChanged: return TEXT("Media Parsed Changed");
		case ELibvlcEventType::MediaFreed: return TEXT("Media Freed");
		case ELibvlcEventType::MediaStateChanged: return FString::Printf(TEXT("Media State Changed: %s"), *StateToString(Event->Descriptor.MediaStateChanged.NewState));
		case ELibvlcEventType::MediaSubItemTreeAdded: return TEXT("Media Sub Item Tree Added");
				
		case ELibvlcEventType::MediaPlayerMediaChanged: return TEXT("Player Media Changed");
		case ELibvlcEventType::MediaPlayerNothingSpecial: return TEXT("Player Nothing Special");
		case ELibvlcEventType::MediaPlayerOpening: return TEXT("Player Opening");
		case ELibvlcEventType::MediaPlayerBuffering: return FString::Printf(TEXT("Player Buffering: %f"), Event->Descriptor.MediaPlayerBuffering.NewCache);
		case ELibvlcEventType::MediaPlayerPlaying: return TEXT("Player Playing");
		case ELibvlcEventType::MediaPlayerPaused: return TEXT("Player Paused");
		case ELibvlcEventType::MediaPlayerStopped: return TEXT("Player Stopped");
		case ELibvlcEventType::MediaPlayerForward: return TEXT("Player Forward");
		case ELibvlcEventType::MediaPlayerBackward: return TEXT("Player Backward");
		case ELibvlcEventType::MediaPlayerEndReached: return TEXT("Player End Reached");
		case ELibvlcEventType::MediaPlayerEncounteredError: return TEXT("Player Encountered Error");
		case ELibvlcEventType::MediaPlayerTimeChanged: return FString::Printf(TEXT("Player Time Changed: %s"), *FTimespan::FromMilliseconds(Event->Descriptor.MediaPlayerTimeChanged.NewTime).ToString());
		case ELibvlcEventType::MediaPlayerPositionChanged: return FString::Printf(TEXT("Position Changed: %f"), Event->Descriptor.MediaPlayerPositionChanged.NewPosition);
		case ELibvlcEventType::MediaPlayerSeekableChanged: return FString::Printf(TEXT("Player Seekable Changed: %s"), Event->Descriptor.MediaPlayerSeekableChanged.new_seekable ? *FCoreTexts::Get().True.ToString() : *FCoreTexts::Get().False.ToString());
		case ELibvlcEventType::MediaPlayerPausableChanged: return FString::Printf(TEXT("Player Pausable Changed: %s"), Event->Descriptor.MediaPlayerPausableChanged.NewPausable ? *FCoreTexts::Get().True.ToString() : *FCoreTexts::Get().False.ToString());
		case ELibvlcEventType::MediaPlayerTitleChanged: return FString::Printf(TEXT("Player Title Changed: %s"), Event->Descriptor.MediaPlayerTitleChanged.NewTitle);
		case ELibvlcEventType::MediaPlayerSnapshotTaken: return TEXT("Player Snapshot Taken");
		case ELibvlcEventType::MediaPlayerLengthChanged: return FString::Printf(TEXT("Player Length Changed: %i"), Event->Descriptor.MediaPlayerLengthChanged.NewLength);
		case ELibvlcEventType::MediaPlayerVout: return FString::Printf(TEXT("Player Vout: %i"), Event->Descriptor.MediaPlayerVout.NewCount);
		case ELibvlcEventType::MediaPlayerScrambledChanged: return FString::Printf(TEXT("Player Scambled Changed: %s"), Event->Descriptor.MediaPlayerScrambledChanged.NewScrambled ? *FCoreTexts::Get().True.ToString() : *FCoreTexts::Get().False.ToString());
		case ELibvlcEventType::MediaPlayerESAdded: return TEXT("Player ES Added");
		case ELibvlcEventType::MediaPlayerESDeleted: return TEXT("Player ES Deleted");
		case ELibvlcEventType::MediaPlayerESSelected: return TEXT("Player ES Selected");
				
		case ELibvlcEventType::MediaListItemAdded: return TEXT("Media List Item Added");
		case ELibvlcEventType::MediaListWillAddItem: return TEXT("Media List Will Add Item");
		case ELibvlcEventType::MediaListItemDeleted: return TEXT("Media List Item Deleted");
		case ELibvlcEventType::MediaListWillDeleteItem: return TEXT("Media List Will Delete Item");
		case ELibvlcEventType::MediaListEndReached: return TEXT("Media List End Reached");

		case ELibvlcEventType::MediaListViewItemAdded: return TEXT("Media List View Item Added");
		case ELibvlcEventType::MediaListViewWillAddItem: return TEXT("Media List View Will Add Item");
		case ELibvlcEventType::MediaListViewItemDeleted: return TEXT("Media List View Item Deleted");
		case ELibvlcEventType::MediaListViewWillDeleteItem: return TEXT("Media List View Will Delete Item");

		case ELibvlcEventType::MediaListPlayerPlayed: return TEXT("Media List Player Played");
		case ELibvlcEventType::MediaListPlayerNextItemSet: return TEXT("Media List Player Next Item Set");
		case ELibvlcEventType::MediaListPlayerStopped: return TEXT("Media List Player Stopped");

		case ELibvlcEventType::MediaDiscovererStarted: return TEXT("Media Discovery Started");
		case ELibvlcEventType::MediaDiscovererEnded: return TEXT("Media Discovery Ended");

		case ELibvlcEventType::VlmMediaAdded: return TEXT("VLM Media Added");
		case ELibvlcEventType::VlmMediaRemoved: return TEXT("VLM Media Removed");
		case ELibvlcEventType::VlmMediaChanged: return TEXT("VLM Media Changed");
		case ELibvlcEventType::VlmMediaInstanceStarted: return TEXT("VLM Instance Started");
		case ELibvlcEventType::VlmMediaInstanceStopped: return TEXT("VLM Instance Stopped");
		case ELibvlcEventType::VlmMediaInstanceStatusInit: return TEXT("VLM Instance Status Init");
		case ELibvlcEventType::VlmMediaInstanceStatusOpening: return TEXT("VLM Instance Status Opening");
		case ELibvlcEventType::VlmMediaInstanceStatusPlaying: return TEXT("VLM Instance Status Playing");
		case ELibvlcEventType::VlmMediaInstanceStatusPause: return TEXT("VLM Instance Status Paused");
		case ELibvlcEventType::VlmMediaInstanceStatusEnd: return TEXT("VLM Instance Status End");
		case ELibvlcEventType::VlmMediaInstanceStatusError: return TEXT("VLM Instance Status Error");

		default:
			return FString::Printf(TEXT("Unknown event %i"), (int32)Event->Type);
		}
	}


	FString StateToString(ELibvlcState State)
	{
		switch (State)
		{
		case ELibvlcState::NothingSpecial: return TEXT("Nothing Special");
		case ELibvlcState::Opening: return TEXT("Opening");
		case ELibvlcState::Buffering: return TEXT("Buffering");
		case ELibvlcState::Playing: return TEXT("Playing");
		case ELibvlcState::Paused: return TEXT("Paused");
		case ELibvlcState::Stopped: return TEXT("Stopped");
		case ELibvlcState::Ended: return TEXT("Ended");
		case ELibvlcState::Error: return TEXT("Error");
		default:
			return FString::Printf(TEXT("Unknown state %i"), (int32)State);
		}
	}
}
