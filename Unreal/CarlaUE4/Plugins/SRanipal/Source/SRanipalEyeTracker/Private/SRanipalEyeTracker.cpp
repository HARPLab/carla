//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========

#include "SRanipalEyeTracker.h"
#include "SRanipal_Eyes_Enums.h"
#include "SRanipalEye_Core.h"
#include "SRanipalEye_Framework.h"
#include "SRanipalSettings.h"

#define LOCTEXT_NAMESPACE "FSRanipalEyeTrackerModule"

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSRanipalEyeTrackerModule, SRanipalEyeTracker)

FSRanipalEyeTracker::FSRanipalEyeTracker()
{
	// Start SRanipalEye framework
	if (GetDefault<USRanipalSettings>()->EnableEyeByDefault)
	{
		SRanipalEye_Framework::Instance()->StartFramework(GetDefault<USRanipalSettings>()->EyeVersion);
	}
}

FSRanipalEyeTracker::~FSRanipalEyeTracker()
{
	// Stop SRanipalEye framework
	SRanipalEye_Framework::Instance()->StopFramework();
	SRanipalEye_Framework::Instance()->DestroyEyeFramework();
}

bool FSRanipalEyeTracker::GetEyeTrackerGazeData(FEyeTrackerGazeData & OutGazeData) const
{
	bool result = SRanipalEye_Core::Instance()->GetGazeRay(GazeIndex::COMBINE, OutGazeData.GazeOrigin, OutGazeData.GazeDirection);
	return result;
}

bool FSRanipalEyeTracker::GetEyeTrackerStereoGazeData(FEyeTrackerStereoGazeData & OutGazeData) const
{
	bool resultLeft = SRanipalEye_Core::Instance()->GetGazeRay(GazeIndex::LEFT, OutGazeData.LeftEyeOrigin, OutGazeData.LeftEyeDirection);
	bool resultRight = SRanipalEye_Core::Instance()->GetGazeRay(GazeIndex::RIGHT, OutGazeData.RightEyeOrigin, OutGazeData.RightEyeDirection);

	return resultLeft && resultRight;
}

EEyeTrackerStatus FSRanipalEyeTracker::GetEyeTrackerStatus() const
{
	switch (SRanipalEye_Framework::Instance()->GetStatus())
	{
	case SRanipalEye_Framework::FrameworkStatus::WORKING:
		return EEyeTrackerStatus::Tracking;
	default:
		return EEyeTrackerStatus::NotConnected;
	}
	
}

bool FSRanipalEyeTracker::IsStereoGazeDataAvailable() const
{
	switch (SRanipalEye_Framework::Instance()->GetStatus())
	{
	case SRanipalEye_Framework::FrameworkStatus::WORKING:
		return true;
	default:
		return false;
	}
}

bool FSRanipalEyeTrackerModule::IsEyeTrackerConnected() const
{
	IEyeTracker* EyeTracker = SRanipalEyeTracker.Get();
	if (EyeTracker)
	{
		EEyeTrackerStatus Status = EyeTracker->GetEyeTrackerStatus();
		if ((Status != EEyeTrackerStatus::NotTracking) && (Status != EEyeTrackerStatus::NotConnected))
		{
			return true;
		}
	}

	return false;
}
