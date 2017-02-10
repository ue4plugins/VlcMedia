// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ModuleInterface.h"
#include "ModuleManager.h"


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
