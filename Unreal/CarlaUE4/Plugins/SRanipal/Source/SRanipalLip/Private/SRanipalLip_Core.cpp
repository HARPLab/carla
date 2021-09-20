//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========


#include "SRanipalLip_Core.h"
#include "SRanipalLip.h"
#include "SRanipal_API_Lip.h"
#include "SRanipalLip_Framework.h"

SRanipalLip_Core* SRanipalLip_Core::SRanipal_LipModule = nullptr;

SRanipalLip_Core::SRanipalLip_Core()
{
	// Define image size.
	ImageWidth = 800; ImageHeight = 400;
	LastUpdateFrame = -1;
	LastUpdateResult = ViveSR::Error::FAILED;
	LipImageBuffer = (uint8*)FMemory::Malloc(ImageWidth * ImageHeight * sizeof(uint8));

	for (int i = 0; i < (int)LipShapeEnum::Max; ++i) {
		Weightings.Add((LipShapeEnum)i, 0.f);
	}

    for (int i = 0; i < (int)LipShapeEnum_v2::Max; ++i) {
        Weightings_v2.Add((LipShapeEnum_v2)i, 0.f);
    }
}

SRanipalLip_Core::~SRanipalLip_Core()
{
	FMemory::Free(LipImageBuffer);
}

SRanipalLip_Core* SRanipalLip_Core::Instance()
{
	if (SRanipal_LipModule == nullptr)
		SRanipal_LipModule = new SRanipalLip_Core();
	return SRanipal_LipModule;
}

void SRanipalLip_Core::DestroyLipModule()
{
	if (SRanipal_LipModule != nullptr) {
		delete SRanipal_LipModule;
		SRanipal_LipModule = nullptr;
	}
}

int SRanipalLip_Core::GetLipData_(ViveSR::anipal::Lip::LipData *data)
{
	return ViveSR::anipal::Lip::GetLipData(data);
}

int SRanipalLip_Core::GetLipData_v2(ViveSR::anipal::Lip::LipData_v2 *data)
{
	return ViveSR::anipal::Lip::GetLipData_v2(data);
}

bool SRanipalLip_Core::GetLipWeightings(TMap<LipShapeEnum, float> &shapes)
{
	bool valid = UpdateData();
	if(valid) shapes = Weightings;
	return valid;
}

bool SRanipalLip_Core::GetLipWeightings_v2(TMap<LipShapeEnum_v2, float> &shapes)
{
	bool valid = UpdateData();
	if(valid) shapes = Weightings_v2;
	return valid;
}

bool SRanipalLip_Core::UpdateTexture(UTexture2D *&texture)
{
	int TexWidth = texture->GetSizeX();
	int TexHeight = texture->GetSizeY();
	bool valid = UpdateData();

	updateRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, TexWidth, TexHeight);
	texture->UpdateTextureRegions(0, 1, updateRegion, static_cast<uint32>(ImageWidth * sizeof(uint8)), sizeof(uint8), LipImageBuffer);
	return valid;
}

bool SRanipalLip_Core::UpdateData()
{
	lipData.image = LipImageBuffer;
    lipData_v2.image = LipImageBuffer;
	if (GFrameCounter != LastUpdateFrame) {
		LastUpdateFrame = GFrameCounter;
		switch (SRanipalLip_Framework::Instance()->GetLipVersion())
		{
		case SupportedLipVersion::version1:
			LastUpdateResult = ViveSR::anipal::Lip::GetLipData(&lipData);
			if (LastUpdateResult == ViveSR::Error::WORK) {
				for (int i = 0; i < (int)LipShapeEnum::Max; ++i) {
					Weightings[(LipShapeEnum)i] = lipData.prediction_data.blend_shape_weight[i];
				}
			}
			break;
		case SupportedLipVersion::version2:
			LastUpdateResult = ViveSR::anipal::Lip::GetLipData_v2(&lipData_v2);
			if (LastUpdateResult == ViveSR::Error::WORK) {
				for (int i = 0; i < (int)LipShapeEnum_v2::Max; ++i) {
					Weightings_v2[(LipShapeEnum_v2)i] = lipData_v2.prediction_data.blend_shape_weight[i];
				}
			}
			break;
		}
	}
	return LastUpdateResult == ViveSR::Error::WORK;
}
