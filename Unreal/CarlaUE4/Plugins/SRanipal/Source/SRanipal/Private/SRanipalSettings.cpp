//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========


#include "SRanipalSettings.h"

USRanipalSettings::USRanipalSettings()
	: EnableEyeByDefault(false)
	, EyeVersion(SupportedEyeVersion::version2)
	, EnableLipByDefault(false)
	, LipVersion(SupportedLipVersion::version2)
{
}

#if WITH_EDITOR
void USRanipalSettings::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

bool USRanipalSettings::CanEditChange(const UProperty * InProperty) const
{
	const bool ParentVal = Super::CanEditChange(InProperty);

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(USRanipalSettings, EyeVersion))
	{
		return ParentVal && EnableEyeByDefault;
	}

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(USRanipalSettings, LipVersion))
	{
		return ParentVal && EnableLipByDefault;
	}

	return ParentVal;
}
#endif


