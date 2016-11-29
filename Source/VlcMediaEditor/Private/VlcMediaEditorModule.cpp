// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "ModuleInterface.h"


/**
 * Implements the VlcMediaEditor module.
 */
class FVlcMediaEditorModule
	: public IModuleInterface
{
public:

	//~ IModuleInterface interface

	virtual void StartupModule() override { }
	virtual void ShutdownModule() override { }
};


IMPLEMENT_MODULE(FVlcMediaEditorModule, VlcMediaEditor);
