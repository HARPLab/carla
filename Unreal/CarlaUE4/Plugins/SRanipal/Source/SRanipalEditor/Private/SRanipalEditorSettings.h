//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========

#pragma once

#include "CoreMinimal.h"
#include "SRanipalSettings.h"
#include "SRanipalEditorSettings.generated.h"

/**
 * Implements the settings for the SRanipal.
 */
UCLASS()
class SRANIPALEDITOR_API USRanipalEditorSettings : public USRanipalSettings
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
};
