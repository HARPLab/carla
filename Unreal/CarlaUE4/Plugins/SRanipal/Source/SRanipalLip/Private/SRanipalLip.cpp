//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========

#include "SRanipalLip.h"
#include "SRanipalSettings.h"
#include "SRanipalLip_Framework.h"

DEFINE_LOG_CATEGORY(LogSRanipalLip);
#define LOCTEXT_NAMESPACE "FSRanipalLipModule"

void FSRanipalLipModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	if (GetDefault<USRanipalSettings>()->EnableLipByDefault)
	{
		SRanipalLip_Framework::Instance()->StartFramework(GetDefault<USRanipalSettings>()->LipVersion);
	}
}

void FSRanipalLipModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	SRanipalLip_Framework::Instance()->StopFramework();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSRanipalLipModule, SRanipalLip)