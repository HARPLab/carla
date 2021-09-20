//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

enum ModuleStatus {
	WORK = 0,
	/** The Unreal windows api can't find the DLLs, please check DLL files. */
	STARTUP_ERROR_DLL_LOST = 1,
	/** The module faild to load DLL when ModuleStartUp. Check the DLL files. */
	STARTUP_ERROR_LOAD_DLL_FAILED = 2,
};

class FSRanipalModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
* Singleton-like access to this module's interface.  This is just for convenience!
* Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
*
* @return Returns singleton instance, loading the module on demand if needed
*/
	static inline FSRanipalModule& Get()
	{
		return FModuleManager::LoadModuleChecked< FSRanipalModule >("SRanipal");
	}

	/**
	* Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	*
	* @return True if the module is loaded and ready to use
	*/
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("SRanipal");
	}

private:
	FString LibraryPath, BaseDir;
	TArray<FString> libNames;
	TArray<void*> DllHandles;
	bool LoadLibraryDLL(const FString& libName, void*& handle);
};
