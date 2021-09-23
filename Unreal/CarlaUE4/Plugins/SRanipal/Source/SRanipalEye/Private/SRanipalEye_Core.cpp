//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========


#include "SRanipalEye_Core.h"
#include "SRanipal_API_Eye.h"
#include "SRanipalEye_Framework.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"

using namespace ViveSR::anipal;

SRanipalEye_Core* SRanipalEye_Core::SRanipal_EyeModule = nullptr;

SRanipalEye_Core::SRanipalEye_Core()
{
	LastUpdateFrame = -1;
	LastUpdateResult = ViveSR::Error::FAILED;
	for (int i = 0; i < (int)EyeShape::Max; ++i) {
		Weightings.Add((EyeShape)i, 0.f);
	}
}

SRanipalEye_Core::~SRanipalEye_Core() {}

SRanipalEye_Core* SRanipalEye_Core::Instance()
{
	if (SRanipal_EyeModule == nullptr)
		SRanipal_EyeModule = new SRanipalEye_Core();
	return SRanipal_EyeModule;
}

void SRanipalEye_Core::DestroyEyeModule()
{
	if (SRanipal_EyeModule != nullptr) {
		delete SRanipal_EyeModule;
		SRanipal_EyeModule = nullptr;
	}
}

int SRanipalEye_Core::GetEyeData_(ViveSR::anipal::Eye::EyeData * data)
{
	return Eye::GetEyeData(data);
}

int SRanipalEye_Core::GetEyeData_v2(ViveSR::anipal::Eye::EyeData_v2 * data)
{
	return Eye::GetEyeData_v2(data);
}

int SRanipalEye_Core::SetEyeParameter_(ViveSR::anipal::Eye::EyeParameter parameter)
{
	return Eye::SetEyeParameter(parameter);
}

int SRanipalEye_Core::GetEyeParameter_(ViveSR::anipal::Eye::EyeParameter *parameter)
{
	return Eye::GetEyeParameter(parameter);
}

int SRanipalEye_Core::LaunchEyeCalibration_(void * callback)
{
	return Eye::LaunchEyeCalibration(callback);
}

bool SRanipalEye_Core::GetVerboseData(ViveSR::anipal::Eye::VerboseData &data)
{
	bool update = UpdateData();
	switch (SRanipalEye_Framework::Instance()->GetEyeVersion())
	{
	case SupportedEyeVersion::version1:
		data = EyeData_.verbose_data;
		break;
	case SupportedEyeVersion::version2:
		data = EyeData_v2.verbose_data;
		break;
	}
	return update;
}

bool SRanipalEye_Core::GetEyeOpenness(EyeIndex eye, float &openness)
{
	bool valid = false;
	if (SRanipalEye_Framework::Instance()->GetStatus() == SRanipalEye_Framework::FrameworkStatus::NOT_SUPPORT) {
		valid = true;
		openness = 1.0f;
	}
	else {
		UpdateData();
		
		Eye::SingleEyeData eyeData;
		switch (SRanipalEye_Framework::Instance()->GetEyeVersion())
		{
		case SupportedEyeVersion::version1:
			eyeData = eye == EyeIndex::LEFT ? EyeData_.verbose_data.left : EyeData_.verbose_data.right;
			break;
		case SupportedEyeVersion::version2:
			eyeData = eye == EyeIndex::LEFT ? EyeData_v2.verbose_data.left : EyeData_v2.verbose_data.right;
			break;
		}
		
		valid = eyeData.GetValidity(Eye::SingleEyeDataValidity::SINGLE_EYE_DATA_EYE_OPENNESS_VALIDITY);
		openness = valid ? eyeData.eye_openness : 0.f;
	}
	return valid;
}

bool SRanipalEye_Core::GetPupilPosition(EyeIndex eye, FVector2D &position)
{
	bool valid = false;
	if (SRanipalEye_Framework::Instance()->GetStatus() == SRanipalEye_Framework::FrameworkStatus::NOT_SUPPORT) {
		valid = true;
		position.Set(0.5f, 0.5f);
	}
	else {
		UpdateData();
		
		Eye::SingleEyeData eyeData;
		switch (SRanipalEye_Framework::Instance()->GetEyeVersion())
		{
			case SupportedEyeVersion::version1:
				eyeData = eye == EyeIndex::LEFT ? EyeData_.verbose_data.left : EyeData_.verbose_data.right;
				break;
			case SupportedEyeVersion::version2:
				eyeData = eye == EyeIndex::LEFT ? EyeData_v2.verbose_data.left : EyeData_v2.verbose_data.right;
				break;
		}

		valid = eyeData.GetValidity(Eye::SingleEyeDataValidity::SINGLE_EYE_DATA_PUPIL_POSITION_IN_SENSOR_AREA_VALIDITY);
		if (valid) {
			position.Set(eyeData.pupil_position_in_sensor_area.X * 2 - 1,
				eyeData.pupil_position_in_sensor_area.Y * -2 + 1);
		}
		else
			position.Set(0.f, 0.f);
	}
	return valid;
}

bool SRanipalEye_Core::GetGazeRay(GazeIndex gazeIndex, FVector &origin, FVector &direction)
{
	bool valid = false;
	origin = FVector::ZeroVector;
	direction = FVector::ForwardVector;
	if (SRanipalEye_Framework::Instance()->GetStatus() == SRanipalEye_Framework::FrameworkStatus::NOT_SUPPORT) {
		origin = SRanipalEye_Framework::Instance()->GetCameraPosition();
		valid = true;
	}
	else {
		UpdateData();

		Eye::SingleEyeData singeEyeDatas[2];
		switch (SRanipalEye_Framework::Instance()->GetEyeVersion())
		{
			case SupportedEyeVersion::version1:
				singeEyeDatas[(int)GazeIndex::LEFT] = EyeData_.verbose_data.left;
				singeEyeDatas[(int)GazeIndex::RIGHT] = EyeData_.verbose_data.right;
				break;
			case SupportedEyeVersion::version2:
				singeEyeDatas[(int)GazeIndex::LEFT] = EyeData_v2.verbose_data.left;
				singeEyeDatas[(int)GazeIndex::RIGHT] = EyeData_v2.verbose_data.right;
				break;
		}
		
		if (gazeIndex == GazeIndex::COMBINE)
		{
			valid = singeEyeDatas[(int)GazeIndex::LEFT].GetValidity(Eye::SingleEyeDataValidity::SINGLE_EYE_DATA_GAZE_DIRECTION_VALIDITY)
				&& singeEyeDatas[(int)GazeIndex::RIGHT].GetValidity(Eye::SingleEyeDataValidity::SINGLE_EYE_DATA_GAZE_DIRECTION_VALIDITY);
			if (valid)
			{
				origin = (singeEyeDatas[(int)GazeIndex::LEFT].gaze_origin_mm + singeEyeDatas[(int)GazeIndex::RIGHT].gaze_origin_mm) / 1000 / 2;
				direction = (singeEyeDatas[(int)GazeIndex::LEFT].gaze_direction_normalized + singeEyeDatas[(int)GazeIndex::RIGHT].gaze_direction_normalized) / 2;
				direction.X *= -1;
				CovertToUnrealLocation(origin);
				ApplyUnrealWorldToMeterScale(origin);
				CovertToUnrealLocation(direction);
			}
		}
		else if (gazeIndex == GazeIndex::LEFT || gazeIndex == GazeIndex::RIGHT)
		{
			valid = singeEyeDatas[(int)gazeIndex].GetValidity(Eye::SingleEyeDataValidity::SINGLE_EYE_DATA_GAZE_DIRECTION_VALIDITY);
			if (valid)
			{
				origin = singeEyeDatas[(int)gazeIndex].gaze_origin_mm / 1000;
				direction = singeEyeDatas[(int)gazeIndex].gaze_direction_normalized;
				direction.X *= -1;
				CovertToUnrealLocation(origin);
				ApplyUnrealWorldToMeterScale(origin);
				CovertToUnrealLocation(direction);
			}
		}
	}
	return valid;
}

bool SRanipalEye_Core::GetEyeWeightings(TMap<EyeShape, float> &shape)
{
	bool update = UpdateData();
	float openness[2];
	bool valid[2];
	FVector2D pupilPosition[2];
	for (int index = 0; index < (int)EyeIndex::MAX; ++index) {
		GetEyeOpenness((EyeIndex)index, openness[index]);
		valid[index] = GetPupilPosition((EyeIndex)index, pupilPosition[(int)index]);
	}

	float weighting_up[3] = { FGenericPlatformMath::Max(pupilPosition[(int)GazeIndex::LEFT].Y, 0.f),
							  FGenericPlatformMath::Max(pupilPosition[(int)GazeIndex::RIGHT].Y, 0.f), 0.f };
	float weighting_down[3] = { -FGenericPlatformMath::Max(pupilPosition[(int)GazeIndex::LEFT].Y, 0.f),
								-FGenericPlatformMath::Max(pupilPosition[(int)GazeIndex::RIGHT].Y, 0.f), 0.f };
	float weighting_left[3] = { -FGenericPlatformMath::Max(pupilPosition[(int)GazeIndex::LEFT].X, 0.f),
								-FGenericPlatformMath::Max(pupilPosition[(int)GazeIndex::RIGHT].X, 0.f), 0.f };
	float weighting_right[3] = { FGenericPlatformMath::Max(pupilPosition[(int)GazeIndex::LEFT].X, 0.f),
								 FGenericPlatformMath::Max(pupilPosition[(int)GazeIndex::RIGHT].X, 0.f), 0.f };
	weighting_up[(int)GazeIndex::COMBINE] = (weighting_up[(int)GazeIndex::LEFT] + weighting_up[(int)GazeIndex::RIGHT]) / 2;
	weighting_down[(int)GazeIndex::COMBINE] = (weighting_down[(int)GazeIndex::LEFT] + weighting_down[(int)GazeIndex::RIGHT]) / 2;
	weighting_left[(int)GazeIndex::COMBINE] = (weighting_left[(int)GazeIndex::LEFT] + weighting_left[(int)GazeIndex::RIGHT]) / 2;
	weighting_right[(int)GazeIndex::COMBINE] = (weighting_right[(int)GazeIndex::LEFT] + weighting_right[(int)GazeIndex::RIGHT]) / 2;

	for (int i = 0; i < (int)EyeShape::Max; ++i) {
		Weightings.Add((EyeShape)i, 0.f);
	}

	Weightings[EyeShape::Eye_Left_Blink] = 1 - openness[(int)EyeIndex::LEFT];
	Weightings[EyeShape::Eye_Right_Blink] = 1 - openness[(int)EyeIndex::RIGHT];

	if (valid[(int)EyeIndex::LEFT] && valid[(int)EyeIndex::RIGHT]) {
		FVector combined_gaze_origin, combined_gaze_direction;
		GetGazeRay(GazeIndex::COMBINE, combined_gaze_origin, combined_gaze_direction);
		FVector gaze_direction = combined_gaze_direction - combined_gaze_origin;
		gaze_direction.Y = 0.0f;
		gaze_direction.Normalize(0.f);
		FVector gaze_axis_forward = FVector(1.0, 0.f, 0.f);
		float z_weight = FGenericPlatformMath::Acos(FVector::DotProduct(gaze_direction, gaze_axis_forward));
	
		Weightings[EyeShape::Eye_Left_Down] = gaze_direction.Z < 0 ? z_weight : 0;
		Weightings[EyeShape::Eye_Left_Left] = weighting_left[(int)GazeIndex::COMBINE];
		Weightings[EyeShape::Eye_Left_Right] = weighting_right[(int)GazeIndex::COMBINE];
		switch (SRanipalEye_Framework::Instance()->GetEyeVersion())
		{
		case SupportedEyeVersion::version1:
			Weightings[EyeShape::Eye_Left_Up] = gaze_direction.Z < 0 ? 0 : z_weight;
			Weightings[EyeShape::Eye_Left_Wide] = Weightings[EyeShape::Eye_Left_Up];
			break;
		case SupportedEyeVersion::version2:
			Weightings[EyeShape::Eye_Left_Up] = EyeData_v2.expression_data.left.eye_wide;
			Weightings[EyeShape::Eye_Left_Wide] = EyeData_v2.expression_data.left.eye_wide;
			break;
		}

		Weightings[EyeShape::Eye_Right_Down] = gaze_direction.Z < 0 ? z_weight : 0;
		Weightings[EyeShape::Eye_Right_Left] = weighting_left[(int)GazeIndex::COMBINE];
		Weightings[EyeShape::Eye_Right_Right] = weighting_right[(int)GazeIndex::COMBINE];
		switch (SRanipalEye_Framework::Instance()->GetEyeVersion())
		{
		case SupportedEyeVersion::version1:
			Weightings[EyeShape::Eye_Right_Up] = gaze_direction.Z < 0 ? 0 : z_weight;
			Weightings[EyeShape::Eye_Right_Wide] = Weightings[EyeShape::Eye_Left_Up];
			break;
		case SupportedEyeVersion::version2:
			Weightings[EyeShape::Eye_Right_Up] = EyeData_v2.expression_data.right.eye_wide;
			Weightings[EyeShape::Eye_Right_Wide] = EyeData_v2.expression_data.right.eye_wide;
			break;
		}
	}
	else if (valid[(int)EyeIndex::LEFT]) {
		switch (SRanipalEye_Framework::Instance()->GetEyeVersion())
		{
			case SupportedEyeVersion::version1:
				Weightings[EyeShape::Eye_Left_Up] = weighting_up[(int)GazeIndex::LEFT];
				Weightings[EyeShape::Eye_Left_Down] = weighting_down[(int)GazeIndex::LEFT];
				Weightings[EyeShape::Eye_Left_Left] = weighting_left[(int)GazeIndex::LEFT];
				Weightings[EyeShape::Eye_Left_Right] = weighting_right[(int)GazeIndex::LEFT];
				Weightings[EyeShape::Eye_Left_Wide] = Weightings[EyeShape::Eye_Left_Up];			
				break;
			case SupportedEyeVersion::version2:
				FVector combined_gaze_origin, combined_gaze_direction;
				GetGazeRay(GazeIndex::COMBINE, combined_gaze_origin, combined_gaze_direction);
				FVector gaze_direction = combined_gaze_direction - combined_gaze_origin;
				gaze_direction.Y = 0.0f;
				gaze_direction.Normalize(0.f);
				FVector gaze_axis_forward = FVector(1.0, 0.f, 0.f);
				float z_weight = FGenericPlatformMath::Acos(FVector::DotProduct(gaze_direction, gaze_axis_forward));
				Weightings[EyeShape::Eye_Left_Up] = EyeData_v2.expression_data.left.eye_wide;
				Weightings[EyeShape::Eye_Left_Down] = gaze_direction.Z < 0 ? z_weight : 0;//weighting_down[(int)GazeIndex::LEFT];
				Weightings[EyeShape::Eye_Left_Left] = weighting_left[(int)GazeIndex::LEFT];
				Weightings[EyeShape::Eye_Left_Right] = weighting_right[(int)GazeIndex::LEFT];
				Weightings[EyeShape::Eye_Left_Wide] = Weightings[EyeShape::Eye_Left_Up];
				break;
		}		
	}
	else if (valid[(int)EyeIndex::RIGHT]) {
		switch (SRanipalEye_Framework::Instance()->GetEyeVersion())
		{
		case SupportedEyeVersion::version1:
			Weightings[EyeShape::Eye_Right_Up] = weighting_up[(int)GazeIndex::RIGHT];
			Weightings[EyeShape::Eye_Right_Down] = weighting_down[(int)GazeIndex::RIGHT];
			Weightings[EyeShape::Eye_Right_Left] = weighting_left[(int)GazeIndex::RIGHT];
			Weightings[EyeShape::Eye_Right_Right] = weighting_right[(int)GazeIndex::RIGHT];
			Weightings[EyeShape::Eye_Right_Wide] = Weightings[EyeShape::Eye_Right_Up];
			break;
		case SupportedEyeVersion::version2:
			FVector combined_gaze_origin, combined_gaze_direction;
			GetGazeRay(GazeIndex::COMBINE, combined_gaze_origin, combined_gaze_direction);
			FVector gaze_direction = combined_gaze_direction - combined_gaze_origin;
			gaze_direction.Y = 0.0f;
			gaze_direction.Normalize(0.f);
			FVector gaze_axis_forward = FVector(1.0, 0.f, 0.f);
			float z_weight = FGenericPlatformMath::Acos(FVector::DotProduct(gaze_direction, gaze_axis_forward));
			Weightings[EyeShape::Eye_Right_Up] = EyeData_v2.expression_data.right.eye_wide;
			Weightings[EyeShape::Eye_Right_Down] = gaze_direction.Z < 0 ? z_weight : 0;//weighting_down[(int)GazeIndex::RIGHT];
			Weightings[EyeShape::Eye_Right_Left] = weighting_left[(int)GazeIndex::RIGHT];
			Weightings[EyeShape::Eye_Right_Right] = weighting_right[(int)GazeIndex::RIGHT];
			Weightings[EyeShape::Eye_Right_Wide] = EyeData_v2.expression_data.right.eye_wide;
			break;
		}	
	}
	shape = Weightings;
	return update;
}

bool SRanipalEye_Core::Focus(GazeIndex gazeIndex, float maxDistance, float radius, APlayerCameraManager* playerCamera, TEnumAsByte<ECollisionChannel> TraceChannel, FFocusInfo &focusInfo, FVector &gazeOrigin, FVector &gazeTarget)
{
	FVector CameraGazeOrigin, CameraGazeDirection;
	FVector RayCastOrigin, RayCastDirection;
	FVector PlayerMainCameraLocation;
	FRotator PlayerMainCameraRotation;

	FCollisionQueryParams traceParam;
	FHitResult hitResult;
	bool hit = false;
	UE_LOG(LogTemp, Log, TEXT("Got here"));

	//FEyeTrackerStereoGazeData outGazeData;
	//if (UEyeTrackerFunctionLibrary::GetGazeData(outGazeData)) {}
	if (GetGazeRay(GazeIndex::COMBINE, CameraGazeOrigin, CameraGazeDirection)) {
		// Find the ray cast origin and target positon.
		PlayerMainCameraLocation = playerCamera->GetCameraLocation();
		PlayerMainCameraRotation = playerCamera->GetCameraRotation();
		RayCastOrigin = PlayerMainCameraLocation;
		RayCastDirection = (PlayerMainCameraRotation.RotateVector(CameraGazeDirection) * maxDistance) + PlayerMainCameraLocation ;
		gazeOrigin = RayCastOrigin;
		gazeTarget = RayCastDirection;

		// Create collision information container.
		traceParam = FCollisionQueryParams(FName("traceParam"), true, playerCamera);
		traceParam.bTraceComplex = true;
		traceParam.bReturnPhysicalMaterial = false;
		hitResult = FHitResult(ForceInit);

		// Single line trace
		if (radius == 0.f) {
			hit = playerCamera->GetWorld()->LineTraceSingleByChannel(
				hitResult,
				RayCastOrigin, RayCastDirection,
				TraceChannel,
				traceParam);
		}
		// Sphear line trace
		else {
			FCollisionShape sphear = FCollisionShape();
			sphear.SetSphere(radius);
			hit = playerCamera->GetWorld()->SweepSingleByChannel(
				hitResult,
				RayCastOrigin, RayCastDirection,
				FQuat(0.f, 0.f, 0.f, 0.f),
				TraceChannel,
				sphear,
				traceParam
			);
			UE_LOG(LogTemp,Warning, TEXT("Sphear line trace"));
		}
		UE_LOG(LogTemp, Log, TEXT("SRanipalDist: %.4f"), hitResult.Distance);
		// If collision occured, fill data into focusInfo.
		if (hit) {
			focusInfo.actor = hitResult.Actor;
			focusInfo.distance = hitResult.Distance;
			focusInfo.point = hitResult.Location;
			focusInfo.normal = hitResult.Normal;
		}
	}
	return hit;
}

bool SRanipalEye_Core::Focus(GazeIndex gazeIndex, float maxDistance, APlayerCameraManager* playerCamera, TEnumAsByte<ECollisionChannel> TraceChannel, FFocusInfo &focusInfo, FVector &gazeOrigin, FVector &gazeTarget)
{
	return SRanipalEye_Core::Focus(gazeIndex, maxDistance, 0, playerCamera, TraceChannel, focusInfo, gazeOrigin, gazeTarget);
}

bool SRanipalEye_Core::Focus(GazeIndex gazeIndex, APlayerCameraManager* playerCamera, TEnumAsByte<ECollisionChannel> TraceChannel, FFocusInfo &focusInfo, FVector &gazeOrigin, FVector &gazeTarget)
{
	return SRanipalEye_Core::Focus(gazeIndex, TNumericLimits<float>::Max(), 0, playerCamera, TraceChannel, focusInfo, gazeOrigin, gazeTarget);
}

bool SRanipalEye_Core::UpdateData()
{
	if (GFrameCounter != LastUpdateFrame) {
		LastUpdateFrame = GFrameCounter;
		switch (SRanipalEye_Framework::Instance()->GetEyeVersion())
		{
		case SupportedEyeVersion::version1:
			LastUpdateResult = GetEyeData(&EyeData_);
			break;
		case SupportedEyeVersion::version2:
			LastUpdateResult = GetEyeData_v2(&EyeData_v2);
			break;
		}
	}
	return LastUpdateResult == ViveSR::Error::WORK;
}

void SRanipalEye_Core::CovertToUnrealLocation(FVector &vector)
{
	FVector temp = vector;
	vector.X = temp.Z;
	vector.Y = temp.X;
	vector.Z = temp.Y;
}

void SRanipalEye_Core::ApplyUnrealWorldToMeterScale(FVector & vector)
{
	float scale = GWorld ? GWorld->GetWorldSettings()->WorldToMeters : 100.f;
	vector *= scale;
}

void SRanipalEye_Core::CovertToUnrealQuaternion(FQuat & quat)
{
	FQuat temp = quat;
	quat.X = temp.Z;
	quat.Y = temp.X;
	quat.Z = temp.Y;
	quat.W = temp.W;
}

