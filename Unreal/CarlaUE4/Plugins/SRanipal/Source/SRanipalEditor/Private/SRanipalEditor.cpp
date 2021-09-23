//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========

#include "SRanipalEditor.h"
#include "ISettingsModule.h"
#include "SRanipalEditorSettings.h"
#include "PropertyEditorModule.h"
#include "SRanipalSettingsCustomization.h"

#define LOCTEXT_NAMESPACE "FSRanipalEditorModule"

void FSRanipalEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// register settings detail panel customization
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(
		USRanipalEditorSettings::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FSRanipalSettingsCustomization::MakeInstance)
	);

	// Register plugin settings.
	ISettingsModule* SettingsModule = FModuleManager::Get().GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "SRanipal", NSLOCTEXT("SRanipal", "SRanipal", "SRanipal"),
			NSLOCTEXT("SRanipal", "ConfigureSRanipalSettings", "Configure SRanipal settings"), GetMutableDefault<USRanipalEditorSettings>());
	}
}

void FSRanipalEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSRanipalEditorModule, SRanipalEditor)