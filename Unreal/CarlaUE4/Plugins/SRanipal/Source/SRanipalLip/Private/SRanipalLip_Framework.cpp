//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========


#include "SRanipalLip_Framework.h"
#include "SRanipal_API.h"
#include "SRanipalLip.h"
#include "SRanipal_API_Lip.h"

SRanipalLip_Framework* SRanipalLip_Framework::Framework = nullptr;

SRanipalLip_Framework::SRanipalLip_Framework()
{
}

SRanipalLip_Framework::~SRanipalLip_Framework()
{
}

SRanipalLip_Framework* SRanipalLip_Framework::Instance()
{
	if (Framework == nullptr) {
		Framework = new SRanipalLip_Framework();
	}
	return Framework;
}

void SRanipalLip_Framework::DestroyLipFramework()
{
	if (Framework != nullptr) {
		delete Framework;
		Framework = nullptr;
	}
}

void SRanipalLip_Framework::StartFramework(SupportedLipVersion version)
{
	if (Status != FrameworkStatus::WORKING) {
		Status = FrameworkStatus::START;

		LipVersion = version;
		if (LipVersion == SupportedLipVersion::version1) {
			int* config = 0;
			int result = ViveSR::anipal::Initial(ViveSR::anipal::Lip::ANIPAL_TYPE_LIP, config);
			if (result != ViveSR::Error::WORK) {
				UE_LOG(LogSRanipalLip, Error, TEXT("[SRanipal] Initial %s failed: %d"), *AnipalTypeName, result);
			}
			else {
				UE_LOG(LogSRanipalLip, Log, TEXT("[SRanipal] Initial %s success."), *AnipalTypeName);
			}
		}
		else {
			int* config = 0;
			int result = ViveSR::anipal::Initial(ViveSR::anipal::Lip::ANIPAL_TYPE_LIP_V2, config);
			if (result != ViveSR::Error::WORK) {
				UE_LOG(LogSRanipalLip, Error, TEXT("[SRanipal] Initial %s failed: %d"), *AnipalTypeName_v2, result);
			}
			else {
				UE_LOG(LogSRanipalLip, Log, TEXT("[SRanipal] Initial %s success."), *AnipalTypeName_v2);
			}
		}
		Status = FrameworkStatus::WORKING;

	}
	UE_LOG(LogSRanipalLip, Log, TEXT("[SRanipal] Framework is running."));
}

void SRanipalLip_Framework::StopFramework()
{
	if (Status != FrameworkStatus::STOP) {
		if (LipVersion == SupportedLipVersion::version1) {
			int result = ViveSR::anipal::Release(ViveSR::anipal::Lip::ANIPAL_TYPE_LIP);
			if (result != ViveSR::Error::WORK) {
				UE_LOG(LogSRanipalLip, Error, TEXT("[SRanipal] Release %s failed: %d"), *AnipalTypeName, result);
			}
			else {
				UE_LOG(LogSRanipalLip, Log, TEXT("[SRanipal] Release %s success"), *AnipalTypeName);
			}
		}
		else {
			int result = ViveSR::anipal::Release(ViveSR::anipal::Lip::ANIPAL_TYPE_LIP_V2);
			if (result != ViveSR::Error::WORK) {
				UE_LOG(LogSRanipalLip, Error, TEXT("[SRanipal] Release %s failed: %d"), *AnipalTypeName_v2, result);
			}
			else {
				UE_LOG(LogSRanipalLip, Log, TEXT("[SRanipal] Release %s success"), *AnipalTypeName_v2);
			}
		}
		Status = FrameworkStatus::STOP;
	}
	UE_LOG(LogSRanipalLip, Log, TEXT("[SRanipal] Framework has been stopped."));
}

