//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SRanipal_Eyes_Enums.h"
#include "Lip/SRanipal_Lip_Enums.h"
#include "SRanipalSettings.generated.h"

/**
 * Implements the settings for the SRanipal.
 */
UCLASS(config = Engine, defaultconfig)
class SRANIPAL_API USRanipalSettings : public UObject
{
	GENERATED_BODY()

public:

	USRanipalSettings();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif

	// 
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Eye Settings")
		bool EnableEyeByDefault;

	// 
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Eye Settings")
		SupportedEyeVersion EyeVersion;
	
	//
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Lip Settings")
		bool EnableLipByDefault;
	
	//
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Lip Settings")
		SupportedLipVersion LipVersion;
};
