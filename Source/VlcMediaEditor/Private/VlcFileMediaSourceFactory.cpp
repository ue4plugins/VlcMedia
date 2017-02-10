// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VlcFileMediaSourceFactory.h"

#include "FileMediaSource.h"
#include "Misc/Paths.h"


/* UVlcFileMediaSourceFactory structors
 *****************************************************************************/

UVlcFileMediaSourceFactory::UVlcFileMediaSourceFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Formats.Add(TEXT("3gp;3GP Video Stream"));
	Formats.Add(TEXT("a52;Dolby Digital AC-3 Audio"));
	Formats.Add(TEXT("aac;MPEG-2 Advanced Audio Coding File"));
	Formats.Add(TEXT("asf;ASF Media File"));
	Formats.Add(TEXT("au;Sun Microsystems Audio"));
	Formats.Add(TEXT("avi;Audio Video Interleave File"));
	Formats.Add(TEXT("dts;Digital Theater System File"));
	Formats.Add(TEXT("dv;Digital Video File"));
	Formats.Add(TEXT("flac;Free Lossless Audio Codec"));
	Formats.Add(TEXT("flv;Adobe Flash Video"));
	Formats.Add(TEXT("mkv;Matroska Video"));
	Formats.Add(TEXT("mka;Matroska Audio"));
	Formats.Add(TEXT("mov;Apple QuickTime Movie"));
	Formats.Add(TEXT("mp2;MPEG-1 Audio"));
	Formats.Add(TEXT("mp3;MPEG-2 Audio"));
	Formats.Add(TEXT("mp4;MPEG-4 Movie"));
	Formats.Add(TEXT("mpg;MPEG-2 Movie"));
	Formats.Add(TEXT("nsc;Windows Media Station"));
	Formats.Add(TEXT("nsv;Nullsoft Streaming Video"));
	Formats.Add(TEXT("nut;NUT Multimedia Container"));
	Formats.Add(TEXT("ogm;Ogg Multimedia"));
	Formats.Add(TEXT("ogg;Ogg Multimedia"));
	Formats.Add(TEXT("ra;Real Audio"));
	Formats.Add(TEXT("ram;Real Audio Metadata"));
	Formats.Add(TEXT("rm;Real Media"));
	Formats.Add(TEXT("rmvb;Real Media VBR"));
	Formats.Add(TEXT("rv;Real Video"));
	Formats.Add(TEXT("sdp;Session Description Protocol File"));
	Formats.Add(TEXT("ts;MPEG-2 Transport Stream"));
	Formats.Add(TEXT("tac;True Audio Codec File"));
	Formats.Add(TEXT("tta;True Audio Codec File"));
	Formats.Add(TEXT("ty;Tivo Container"));
	Formats.Add(TEXT("vid;Generic Video File"));
	Formats.Add(TEXT("wav;Wave Audio File"));
	Formats.Add(TEXT("wmv;Windows Media Video"));
	Formats.Add(TEXT("xa;PlayStation Audio File"));

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
