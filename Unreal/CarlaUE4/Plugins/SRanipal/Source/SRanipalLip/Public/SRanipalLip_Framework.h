//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========

#pragma once

#include "CoreMinimal.h"
#include "SRanipal_Lip_Enums.h"

/**
 * 
 */
class SRANIPALLIP_API SRanipalLip_Framework
{
public:
	SRanipalLip_Framework();
	~SRanipalLip_Framework();

	static SRanipalLip_Framework* Instance();
	static void DestroyLipFramework();

	/** Start the framework. */
	void StartFramework(SupportedLipVersion version);
	/** Stop the framework and release all resource. */
	void StopFramework();

	SupportedLipVersion GetLipVersion()
	{
		return LipVersion;
	}

    int GetStatus()
    {
        return Status;
    }
    enum FrameworkStatus {
        STOP,
        START,
        WORKING,
        ERROR,
    };
private:
	int Status;
	SupportedLipVersion LipVersion;
	static SRanipalLip_Framework* Framework;
	const FString AnipalTypeName = "Lip";
	const FString AnipalTypeName_v2 = "Lip_v2";
};
