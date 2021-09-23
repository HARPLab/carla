//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========

#include "SRanipalEye.h"

DEFINE_LOG_CATEGORY(LogSRanipalEye);
#define LOCTEXT_NAMESPACE "FSRanipalEyeModule"

void FSRanipalEyeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FSRanipalEyeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSRanipalEyeModule, SRanipalEye)