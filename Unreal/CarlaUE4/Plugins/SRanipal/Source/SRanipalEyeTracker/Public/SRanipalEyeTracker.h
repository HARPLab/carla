//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========

#pragma once

#include "CoreMinimal.h"
#include "IEyeTrackerModule.h"
#include "IEyeTracker.h"
#include "Modules/ModuleManager.h"

class FSRanipalEyeTracker :
	public IEyeTracker
{
public:
	FSRanipalEyeTracker();
	virtual ~FSRanipalEyeTracker();

private:
	// IEyeTracker
	virtual void SetEyeTrackedPlayer(APlayerController*) override { }
	virtual bool GetEyeTrackerGazeData(FEyeTrackerGazeData& OutGazeData) const override;
	virtual bool GetEyeTrackerStereoGazeData(FEyeTrackerStereoGazeData& OutGazeData) const override;
	virtual EEyeTrackerStatus GetEyeTrackerStatus() const override;
	virtual bool IsStereoGazeDataAvailable() const override;
	//~ IEyeTracker
};

class FSRanipalEyeTrackerModule : public IEyeTrackerModule
{
public:

	static inline FSRanipalEyeTrackerModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FSRanipalEyeTrackerModule>("SRanipalEyeTrackerModule");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("SRanipalEyeTrackerModule");
	}

	virtual FString GetModuleKeyName() const override
	{
		return TEXT("SRanipalEyeTrackerModule");
	}

	virtual bool IsEyeTrackerConnected() const override;

	virtual TSharedPtr< class IEyeTracker, ESPMode::ThreadSafe > CreateEyeTracker() override
	{
		SRanipalEyeTracker = TSharedPtr< class FSRanipalEyeTracker, ESPMode::ThreadSafe >(new FSRanipalEyeTracker);
		return SRanipalEyeTracker;
	}
private:
	TSharedPtr<FSRanipalEyeTracker, ESPMode::ThreadSafe> SRanipalEyeTracker;

	/** IModuleInterface implementation */
	//virtual void StartupModule() override;
	//virtual void ShutdownModule() override;
};
