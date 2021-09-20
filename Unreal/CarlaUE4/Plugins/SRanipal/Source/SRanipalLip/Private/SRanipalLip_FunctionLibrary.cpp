//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========


#include "SRanipalLip_FunctionLibrary.h"
#include "SRanipalLip_Framework.h"
#include "SRanipalSettings.h"
#include "SRanipalLip.h"

void USRanipalLip_FunctionLibrary::StartLipFramework(SupportedLipVersion version)
{
	if (GetDefault<USRanipalSettings>()->EnableLipByDefault)
	{
		UE_LOG(LogSRanipalLip, Warning, TEXT("[SRanipal] This function \"StartLipFramework\" will NOT work if \"EnableLipByDefault\" is enabled in Project settings."));
	}
	else
	{
		SRanipalLip_Framework::Instance()->StartFramework(version);
	}
}

void USRanipalLip_FunctionLibrary::StopLipFramework()
{
	if (GetDefault<USRanipalSettings>()->EnableLipByDefault)
	{
		UE_LOG(LogSRanipalLip, Warning, TEXT("[SRanipal] This function \"StopLipFramework\" will NOT work if \"EnableLipByDefault\" is enabled in Project settings."));
	}
	else
	{
		SRanipalLip_Framework::Instance()->StopFramework();
	}
}

void USRanipalLip_FunctionLibrary::GetLipWeightings(TMap<LipShapeEnum, float> &shapes)
{
	SRanipalLip_Core::Instance()->GetLipWeightings(shapes);
}

void USRanipalLip_FunctionLibrary::GetLipWeightings_v2(TMap<LipShapeEnum_v2, float> &shapes)
{
	SRanipalLip_Core::Instance()->GetLipWeightings_v2(shapes);
}

void USRanipalLip_FunctionLibrary::UpdateTexture(UTexture2D *&texture)
{
	SRanipalLip_Core::Instance()->UpdateTexture(texture);
}