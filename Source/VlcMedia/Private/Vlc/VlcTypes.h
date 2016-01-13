// Copyright 2015 Headcrash Industries LLC. All Rights Reserved.

#pragma once


/** Enumerates event types. */
enum class ELibvlcEventType
{
	MediaMetaChanged = 0,
    MediaSubItemAdded,
    MediaDurationChanged,
    MediaParsedChanged,
    MediaFreed,
    MediaStateChanged,
    MediaSubItemTreeAdded,

    MediaPlayerMediaChanged = 0x100,
    MediaPlayerNothingSpecial,
    MediaPlayerOpening,
    MediaPlayerBuffering,
    MediaPlayerPlaying,
    MediaPlayerPaused,
    MediaPlayerStopped,
    MediaPlayerForward,
    MediaPlayerBackward,
    MediaPlayerEndReached,
    MediaPlayerEncounteredError,
    MediaPlayerTimeChanged,
    MediaPlayerPositionChanged,
    MediaPlayerSeekableChanged,
    MediaPlayerPausableChanged,
    MediaPlayerTitleChanged,
    MediaPlayerSnapshotTaken,
    MediaPlayerLengthChanged,
    MediaPlayerVout,
    MediaPlayerScrambledChanged,
    MediaPlayerESAdded,
    MediaPlayerESDeleted,
    MediaPlayerESSelected,

    MediaListItemAdded = 0x200,
    MediaListWillAddItem,
    MediaListItemDeleted,
    MediaListWillDeleteItem,
    MediaListEndReached,

    MediaListViewItemAdded = 0x300,
    MediaListViewWillAddItem,
    MediaListViewItemDeleted,
    MediaListViewWillDeleteItem,

    MediaListPlayerPlayed = 0x400,
    MediaListPlayerNextItemSet,
    MediaListPlayerStopped,

    MediaDiscovererStarted = 0x500,
    MediaDiscovererEnded,

    VlmMediaAdded = 0x600,
    VlmMediaRemoved,
    VlmMediaChanged,
    VlmMediaInstanceStarted,
    VlmMediaInstanceStopped,
    VlmMediaInstanceStatusInit,
    VlmMediaInstanceStatusOpening,
    VlmMediaInstanceStatusPlaying,
    VlmMediaInstanceStatusPause,
    VlmMediaInstanceStatusEnd,
    VlmMediaInstanceStatusError
};


/** Enumerates logging levels. */
enum class ELibvlcLogLevel
{
	Debug,
	Notice,
	Warning,
	Error
};


/** Enumerates meta data types. */
enum class ELibvlcMeta
{
	Title,
	Artist,
	Genre,
	Copyright,
	Album,
	TrackNumber,
	Description,
	Rating,
	Date,
	Setting,
	URL,
	Language,
	NowPlaying,
	Publisher,
	EncodedBy,
	ArtworkURL,
	TrackID,
	TrackTotal,
	Director,
	Season,
	Episode,
	ShowName,
	Actors,
	AlbumArtist,
	DiscNumber
};


/** Enumerates possible media player states. */
enum class ELibvlcState
{
    NothingSpecial = 0,
    Opening,
    Buffering,
    Playing,
    Paused,
    Stopped,
    Ended,
    Error
};


/** Enumerates known track types. */
enum class ELibvlcTrackType
{
	Unknown,
	Audio,
	Video,
	Text
};


/** Opaque structure representing a LibVLC event manager. */
typedef struct FLibvlcEventManager FLibvlcEventManager;

/** Opaque structure representing a LibVLC instance. */
typedef struct FLibvlcInstance FLibvlcInstance;

/** Opaque structure representing a LibVLC log context. */
typedef struct FLibvlcLog FLibvlcLog;

/** Opaque structure representing a media instance. */
typedef struct FLibvlcMedia FLibvlcMedia;

/** Opaque structure representing a LibVLC media player. */
typedef struct FLibvlcMediaPlayer FLibvlcMediaPlayer;


/**
 * Structure for VLC events.
 */
struct FLibvlcEvent
{
	/** The type of event (see ELibvlcEvents) */
    ELibvlcEventType Type;

	/** The object emitting the event. */
    void *Obj;

    union
    {
        // media descriptor
        struct
        {
            ELibvlcMeta MetaType;
        } MediaMetaChanged;

        struct
        {
            FLibvlcMedia* NewChild;
        } MediaSubitemAdded;
        
		struct
        {
            int64 NewDuration;
        } MediaDurationChanged;

        struct
        {
            int32 NewStatus;
        } MediaParsedChanged;

        struct
        {
            FLibvlcMedia* Md;
        } MediaFreed;

        struct
        {
            ELibvlcState NewState;
        } MediaStateChanged;

		struct
        {
            FLibvlcMedia* Item;
        } MediaSubitemtreeAdded;

        // media instance
        struct
        {
            float NewCache;
        } MediaPlayerBuffering;

        struct
        {
            float NewPosition;
        } MediaPlayerPositionChanged;

        struct
        {
            int64 NewTime;
        } MediaPlayerTimeChanged;

		struct
        {
            int32 NewTitle;
        } MediaPlayerTitleChanged;

		struct
        {
            int32 new_seekable;
        } MediaPlayerSeekableChanged;

		struct
        {
            int32 NewPausable;
        } MediaPlayerPausableChanged;

		struct
        {
            int32 NewScrambled;
        } MediaPlayerScrambledChanged;

		struct
        {
            int32 NewCount;
        } MediaPlayerVout;

        // media list
        struct
        {
            FLibvlcMedia* Item;
            int32 Index;
        } MediaListItemAdded;

        struct
        {
            FLibvlcMedia* Item;
            int32 Index;
        } MediaListWillAddItem;

        struct
        {
            FLibvlcMedia* Item;
            int32 Index;
        } MediaListItemDeleted;

		struct
        {
            FLibvlcMedia* Item;
            int32 Index;
        } MediaListWillDeleteItem;

        // media list player
        struct
        {
            FLibvlcMedia* Item;
        } MediaListPlayerNextItemSet;

        // snapshot taken
        struct
        {
             ANSICHAR* Filename;
        } MediaPlayerSnapshotTaken;

        // Length changed
        struct
        {
            int64 NewLength;
        } MediaPlayerLengthChanged;

        // VLM media
        struct
        {
            const ANSICHAR* MediaName;
            const ANSICHAR* InstanceName;
        } VlmMediaEvent;

        // extra MediaPlayer
        struct
        {
            FLibvlcMedia* NewMedia;
        } MediaPlayerMediaChanged;
    } Descriptor;
};


/**
 * Structure for VLC media statistics.
 */
struct FLibvlcMediaStats
{
	int32 ReadBytes;
	float InputBitrate;
	int32 DemuxReadBytes;
	float DemuxBitrate;
	int32 DemuxCorrupted;
	int32 DemuxDiscontinuity;
	int32 DecodedVideo;
	int32 DecodedAudio;
	int32 DisplayedPictures;
	int32 LostPictures;
	int32 PlayedAbuffers;
	int32 LostAbuffers;
	int32 SentPackets;
	int32 SentBytes;
	float SendBitrate;
};


/**
 * Structure for VLC media tracks.
 */
struct FLibvlcMediaTrack
{
	uint32 Codec;
	uint32 OriginalFourCC;
	int32 Id;
	ELibvlcTrackType Type;
	int32 Profile;
	int32 Level;
};


/**
 * Structure for VLC media track descriptions.
 */
struct FLibvlcTrackDescription
{
	int32 Id;
	ANSICHAR* Name;
	FLibvlcTrackDescription* Next;
};
