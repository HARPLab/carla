//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========

#include "SRanipal.h"

#include "Projects/Public/Interfaces/IPluginManager.h"
#include "Core.h"
#include "Core/Public/Misc/MessageDialog.h"
#include "Core/Public/Misc/Paths.h"
#include "Core/Public/GenericPlatform/GenericPlatformProcess.h"

#define LOCTEXT_NAMESPACE "FSRanipalModule"

void FSRanipalModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	BaseDir = IPluginManager::Get().FindPlugin("SRanipal")->GetBaseDir();
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/Win64"));
	FPlatformProcess::AddDllDirectory(*LibraryPath);

	libNames.Add("SRanipal.dll");
	DllHandles.Init(nullptr, libNames.Num());
	for (int i = 0; i < libNames.Num(); ++i) {
		LoadLibraryDLL(*libNames[i], DllHandles[i]);
		if (!DllHandles[i]) {
			UE_LOG(LogTemp, Error, TEXT("[SRanipal] Load DLL failed. %d"));
			return;
		}
	}
	UE_LOG(LogTemp, Log, TEXT("[SRanipal] Module start up Success."));
}

void FSRanipalModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	for (int i = 0; i < libNames.Num(); ++i) {
		FPlatformProcess::FreeDllHandle(DllHandles[i]);
		DllHandles[i] = nullptr;
	}
	UE_LOG(LogTemp, Log, TEXT("[SRanipal] Module shut down."));
}

bool FSRanipalModule::LoadLibraryDLL(const FString& libName, void*& handle)
{
	FString LoadPath = FPaths::Combine(*LibraryPath, libName);
	handle = !LoadPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LoadPath) : nullptr;
	return handle != nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSRanipalModule, SRanipal)