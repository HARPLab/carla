//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========


#include "SRanipalEye_FunctionLibrary.h"
#include "SRanipalEye_Core.h"
#include "SRanipalSettings.h"
#include "SRanipalEye.h"
#include "SRanipalEye_Framework.h"

void USRanipalEye_FunctionLibrary::StartEyeFramework(SupportedEyeVersion version)
{
	if (GetDefault<USRanipalSettings>()->EnableEyeByDefault)
	{
		UE_LOG(LogSRanipalEye, Warning, TEXT("[SRanipal] This function \"StartEyeFramework\" will NOT work if \"EnableEyeByDefault\" is enabled in Project settings."));
	}
	else
	{
		SRanipalEye_Framework::Instance()->StartFramework(version);
	}
}

void USRanipalEye_FunctionLibrary::StopEyeFramework()
{
	if (GetDefault<USRanipalSettings>()->EnableEyeByDefault)
	{
		UE_LOG(LogSRanipalEye, Warning, TEXT("[SRanipal] This function \"StopEyeFramework\" will NOT work if \"EnableEyeByDefault\" is enabled in Project settings."));
	}
	else
	{
		SRanipalEye_Framework::Instance()->StopFramework();
	}
}


bool USRanipalEye_FunctionLibrary::GetEyeOpenness(EyeIndex eye, float &openness)
{
	return SRanipalEye_Core::Instance()->GetEyeOpenness(eye, openness);
}

bool USRanipalEye_FunctionLibrary::GetPupilPosition(EyeIndex eye, FVector2D &position)
{
	return SRanipalEye_Core::Instance()->GetPupilPosition(eye, position);
}

bool USRanipalEye_FunctionLibrary::GetGazeRay(GazeIndex gazeIndex, FVector &origin, FVector &direction)
{
	return SRanipalEye_Core::Instance()->GetGazeRay(gazeIndex, origin, direction);
}

bool USRanipalEye_FunctionLibrary::Focus(GazeIndex gazeIndex, float maxDistance, float radius, APlayerCameraManager* playerCamera, TEnumAsByte<ECollisionChannel> TraceChannel, FFocusInfo &focusInfo, FVector &gazeOrigin, FVector &gazeTarget)
{
	return SRanipalEye_Core::Instance()->Focus(gazeIndex, maxDistance, radius, playerCamera, TraceChannel, focusInfo, gazeOrigin, gazeTarget);
}

bool USRanipalEye_FunctionLibrary::GetEyeWeightings(TMap<EyeShape, float> &shapes)
{
	return SRanipalEye_Core::Instance()->GetEyeWeightings(shapes);
}