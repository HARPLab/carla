//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========


#include "SRanipalEditorSettings.h"
#include "SRanipalEye_Framework.h"
#include "SRanipalLip_Framework.h"

#if WITH_EDITOR
void USRanipalEditorSettings::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USRanipalSettings, EnableEyeByDefault))
	{
		if (EnableEyeByDefault)
		{
			SRanipalEye_Framework::Instance()->StartFramework(EyeVersion);
		}
		else
		{
			SRanipalEye_Framework::Instance()->StopFramework();
		}
		
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USRanipalSettings, EyeVersion))
	{
		if (EnableEyeByDefault)
		{
			SRanipalEye_Framework::Instance()->StopFramework();
			SRanipalEye_Framework::Instance()->StartFramework(EyeVersion);
		}
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USRanipalSettings, EnableLipByDefault))
	{
		if (EnableLipByDefault)
		{
			SRanipalLip_Framework::Instance()->StartFramework(LipVersion);
		}
		else
		{
			SRanipalLip_Framework::Instance()->StopFramework();
		}
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USRanipalSettings, LipVersion))
	{
		if (EnableLipByDefault)
		{
			SRanipalLip_Framework::Instance()->StopFramework();
			SRanipalLip_Framework::Instance()->StartFramework(LipVersion);
		}
	}
}
#endif