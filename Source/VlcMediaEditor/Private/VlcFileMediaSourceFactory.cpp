// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VlcFileMediaSourceFactory.h"

#include "FileMediaSource.h"
#include "Misc/Paths.h"


/* UVlcFileMediaSourceFactory structors
 *****************************************************************************/

UVlcFileMediaSourceFactory::UVlcFileMediaSourceFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Formats.Add(TEXT("3g2;3GPP2 Multimedia"));
	Formats.Add(TEXT("3ga;3GPP Audio"));
	Formats.Add(TEXT("3gp;3GP Video Stream"));
	Formats.Add(TEXT("3gp2;3GPP Multimedia"));
	Formats.Add(TEXT("3gpp;3GPP Media"));
	Formats.Add(TEXT("669;UNIS Composer 669 Module"));
	Formats.Add(TEXT("a52;Dolby Digital AC-3 Audio"));
	Formats.Add(TEXT("aac;MPEG-2 Advanced Audio Coding"));
	Formats.Add(TEXT("ac3;Audio Codec 3"));
	Formats.Add(TEXT("adt;Audio Data Transport Stream"));
	Formats.Add(TEXT("adts;Audio Data Transport Stream"));
	Formats.Add(TEXT("aif;Audio Interchange File Format"));
	Formats.Add(TEXT("aifc;Compressed Audio Interchange File"));
	Formats.Add(TEXT("aiff;Audio Interchange File Format"));
	Formats.Add(TEXT("amb;Ambisonics B-Format"));
	Formats.Add(TEXT("amr;Adaptive Multi-Rate Codec"));
	Formats.Add(TEXT("amv;Anime Music Video"));
	Formats.Add(TEXT("aob;DVD-Audio Audio Object"));
	Formats.Add(TEXT("ape;AVS Plugin Effects File"));
	Formats.Add(TEXT("asf;ASF Media"));
	Formats.Add(TEXT("au;Sun Microsystems Audio"));
	Formats.Add(TEXT("avi;Audio Video Interleave File"));
	Formats.Add(TEXT("awb;AMR-WB Audio"));
	Formats.Add(TEXT("bik;Bink Video"));
	Formats.Add(TEXT("bin;Binary File"));
	Formats.Add(TEXT("caf;Core Audio"));
	Formats.Add(TEXT("crf;CRF Video"));
	Formats.Add(TEXT("divx;DivX-Encoded Movie"));
	Formats.Add(TEXT("drc;DRM Rights Object"));
	Formats.Add(TEXT("dts;Digital Theater System"));
	Formats.Add(TEXT("dv;Digital Video"));
	Formats.Add(TEXT("evo;SeeVogh Player Video Recording"));
	Formats.Add(TEXT("f4v;Flash MP4 Video"));
	Formats.Add(TEXT("flac;Free Lossless Audio Codec"));
	Formats.Add(TEXT("flv;Adobe Flash Video"));
	Formats.Add(TEXT("gvi;Google Video"));
	Formats.Add(TEXT("gxf;General eXchange Format"));
	Formats.Add(TEXT("iso;Disc Image"));
	Formats.Add(TEXT("it;Impulse Tracker Module"));
	Formats.Add(TEXT("kar;Karaoke MIDI"));
	Formats.Add(TEXT("m1v;MPEG-1 Video"));
	Formats.Add(TEXT("m2t;HDV Video"));
	Formats.Add(TEXT("m2ts;Blu-ray BDAV Video"));
	Formats.Add(TEXT("m2v;MPEG-2 Video"));
	Formats.Add(TEXT("m4a;MPEG-4 Audio"));
	Formats.Add(TEXT("m4b;MPEG-4 Audio Book"));
	Formats.Add(TEXT("m4p;iTunes Music Store Audio"));
	Formats.Add(TEXT("m5p;MachFive Preset File"));
	Formats.Add(TEXT("m4v;iTunes Video"));
	Formats.Add(TEXT("mid;MIDI File"));
	Formats.Add(TEXT("mka;Matroska Audio"));
	Formats.Add(TEXT("mkv;Matroska Video"));
	Formats.Add(TEXT("mlp;Meridian Lossless Packing Audio"));
	Formats.Add(TEXT("mod;Amiga Music Module"));
	Formats.Add(TEXT("mov;Apple QuickTime Movie"));
	Formats.Add(TEXT("mp1;MPEG-1 Layer 1 Audio"));
	Formats.Add(TEXT("mp2;MPEG-1 Audio"));
	Formats.Add(TEXT("mp2v;MPEG-2 Video"));
	Formats.Add(TEXT("mp3;MPEG-2 Audio"));
	Formats.Add(TEXT("mp4;MPEG-4 Movie"));
	Formats.Add(TEXT("mp4v;MPEG-4 Video"));
	Formats.Add(TEXT("mpa;MPEG-2 Audio"));
	Formats.Add(TEXT("mpc;Musepack Compressed Audio"));
	Formats.Add(TEXT("mpe;MPEG Movie"));
	Formats.Add(TEXT("mpeg;MPEG Movie"));
	Formats.Add(TEXT("mpeg1;MPEG-1 Video"));
	Formats.Add(TEXT("mpeg2;MPEG-2 Video"));
	Formats.Add(TEXT("mpeg4;MPEG-4 Video"));
	Formats.Add(TEXT("mpg;MPEG-2 Movie"));
	Formats.Add(TEXT("mpga;MPEG-1 Layer 3 Audio"));
	Formats.Add(TEXT("mpv2;MPEG-2 Video Stream"));
	Formats.Add(TEXT("mts;AVCHD Video"));
	Formats.Add(TEXT("mtv;MTV Video Format"));
	Formats.Add(TEXT("mus;Finale Notation File"));
	Formats.Add(TEXT("mxf;Material Exchange Format"));
	Formats.Add(TEXT("mxg;Miinoto Exchangeable Group File"));
	Formats.Add(TEXT("nsc;Windows Media Station"));
	Formats.Add(TEXT("nsv;Nullsoft Streaming Video"));
	Formats.Add(TEXT("nut;NUT Multimedia Container"));
	Formats.Add(TEXT("nuv;NuppelVideo File"));
	Formats.Add(TEXT("oga;Ogg Vorbis Audio"));
	Formats.Add(TEXT("ogg;Ogg Multimedia"));
	Formats.Add(TEXT("ogm;Ogg Multimedia"));
	Formats.Add(TEXT("ogv;Ogg Video"));
	Formats.Add(TEXT("ogx;Ogg Vorbis Multiplexed Media"));
	Formats.Add(TEXT("oma;Sony OpenMG Music"));
	Formats.Add(TEXT("opus;Opus Audio"));
	Formats.Add(TEXT("qcp;PureVoice Audio"));
	Formats.Add(TEXT("ps;Program Stream Container"));
	Formats.Add(TEXT("ra;Real Audio"));
	Formats.Add(TEXT("ram;Real Audio Metadata"));
	Formats.Add(TEXT("rec;Topfield PVR Recording"));
	Formats.Add(TEXT("rm;Real Media"));
	Formats.Add(TEXT("rmi;RMID MIDI"));
	Formats.Add(TEXT("rmvb;Real Media VBR"));
	Formats.Add(TEXT("rpl;Toribash Replay File"));
	Formats.Add(TEXT("rv;Real Video"));
	Formats.Add(TEXT("s3m;ScreamTracker 3 Module"));
	Formats.Add(TEXT("sdp;Session Description Protocol"));
	Formats.Add(TEXT("sid;Commodore C64 SID File"));
	Formats.Add(TEXT("spx;Speex Audio"));
	Formats.Add(TEXT("ts;MPEG-2 Transport Stream"));
	Formats.Add(TEXT("tak;Tom's Lossless Audio Kompressor"));
	Formats.Add(TEXT("thd;Dolby TrueHD Audio Stream"));
	Formats.Add(TEXT("thp;Wii/GameCube Video"));
	Formats.Add(TEXT("tod;JVC Everio Video Capture"));
	Formats.Add(TEXT("tta;True Audio Codec"));
	Formats.Add(TEXT("tts;Telerik Trainer Session"));
	Formats.Add(TEXT("txd;TXD File"));
	Formats.Add(TEXT("ty;Tivo Container"));
	Formats.Add(TEXT("vid;Generic Video"));
	Formats.Add(TEXT("vob;DVD Video Object"));
	Formats.Add(TEXT("voc;Creative Labs Audio"));
	Formats.Add(TEXT("vqf;TwinVQ Audio"));
	Formats.Add(TEXT("vro;DVD Video Recording Format"));
	Formats.Add(TEXT("w64;Sony Wave64 Audio"));
	Formats.Add(TEXT("wav;Wave Audio"));
	Formats.Add(TEXT("webm;WebM Video"));
	Formats.Add(TEXT("wm;Windows Media"));
	Formats.Add(TEXT("wma;Windows Media Audio"));
	Formats.Add(TEXT("wmv;Windows Media Video"));
	Formats.Add(TEXT("wtv;Windows Recorded TV Show"));
	Formats.Add(TEXT("wv;WavPack Audio"));
	Formats.Add(TEXT("xa;PlayStation Audio"));
	Formats.Add(TEXT("xesc;Expression Encoder Screen Capture"));
	Formats.Add(TEXT("xm;Fasttracker 2 Extended Module"));

	SupportedClass = UFileMediaSource::StaticClass();
	bEditorImport = true;
}


/* UFactory overrides
 *****************************************************************************/

bool UVlcFileMediaSourceFactory::FactoryCanImport(const FString& Filename)
{
	// @hack: disable file extensions that are used in other factories
	// @todo gmp: add support for multiple factories per file extension
	const FString FileExtension = FPaths::GetExtension(Filename);

	return (FileExtension.ToUpper() != FString("WAV"));
}


UObject* UVlcFileMediaSourceFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	UFileMediaSource* MediaSource = NewObject<UFileMediaSource>(InParent, InClass, InName, Flags);
	MediaSource->SetFilePath(CurrentFilename);

	return MediaSource;
}
