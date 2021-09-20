//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========

#pragma once

#include "CoreMinimal.h"
#include "SRanipal_Eyes_Enums.h"
#include "Camera/PlayerCameraManager.h"

/**
 *  @brief The internal static class about eye relative API 
 */
class SRANIPALEYE_API SRanipalEye_Core
{
public:
	SRanipalEye_Core();
	~SRanipalEye_Core();

	static SRanipalEye_Core* Instance();
	static void DestroyEyeModule();

	/**
	* Get the EyeData from Eye engine.
	* @param data ViveSR::anipal::Eye::EyeData
	* @return one of ViveSR::Error
	*/
	int GetEyeData_(ViveSR::anipal::Eye::EyeData *data);

	/**
	* Get the EyeData_v2 from Eye engine.
	* @param data ViveSR::anipal::Eye::EyeData
	* @return one of ViveSR::Error
	*/
	int GetEyeData_v2(ViveSR::anipal::Eye::EyeData_v2 *data);

	/**
	* Set the EyeParameter to Eye engine.
	* @param parameter ViveSR::anipal::Eye::EyeParameter
	* @return one of ViveSR::Error
	*/
	int SetEyeParameter_(ViveSR::anipal::Eye::EyeParameter parameter);
	/**
	* Get the EyeParameter from Eye engine.
	* @param parameter ViveSR::anipal::Eye::EyeParameter
	* @return one of ViveSR::Error
	*/
	int GetEyeParameter_(ViveSR::anipal::Eye::EyeParameter *parameter);
	/**
	* Launch Eye Calibration, an overlay program and receive the result by register the callback function.
	* @param data callback It will be triggered when the calibration process finished but is not supported yet.
	* @return one of ViveSR::Error
	*/
	int LaunchEyeCalibration_(void* callback);

	/**
	* Get the VerboseData.
	* @param data
	* @return true if there is any new data.
	*/
	bool GetVerboseData(ViveSR::anipal::Eye::VerboseData &data);
	/**
	* Get the openness of specific eye.
	* @param[out] openness The eye openness is clamped to the range [0,1]. when openness = 0, eye is closed. when openness = 1 eye is open.
	* @return true if the openess is valid, otherwise false.
	*/
	bool GetEyeOpenness(EyeIndex eye, float &openness);
	/**
	* Get the position of specific pupil.
	* @param eye Specific which eye.
	* @param[out] position The 2d vector specify the pupil's position.
	* @return true if the position is valid, otherwise false.
	*/
	bool GetPupilPosition(EyeIndex eye, FVector2D &position);
	/**
	* Get a dictionary that stores individual weighting with respect to EyeShape.
	* @param[out] weightings A TArray that stores individual weighting with respect to EyeShape.
	* @return true if there is any new data.
	*/
	bool GetEyeWeightings(TMap<EyeShape, float> &shape);

	/**
	* Get the gaze ray of specific eye.
	* @param gazeIndex Specific which eye.
	* @param[out] origin The starting point of the ray in local coordinates.
	* @param[out] direction The direction of the ray.
	* @return true if the gaze ray is valid, otherwise false.
	*/
	bool GetGazeRay(GazeIndex gazeIndex, FVector &origin, FVector &direction);

	/**
	* Get the combined gaze ray and focusInfo when any candidate object was focused on.
	* @param gazeIndex Specific which eye.
	* @param maxDistance the max distance the ray should check for collisions.
	* @param radius The radius of the gaze ray.
	* @param playerCamera The player's camera controller. You can get this by UGameplayStatics::GetPlayerController(UWorld*, playerIndex)->PlayerCameraManager.
	* @param[out] focusInfo If true is returned, focusInfo will contain the information about where the actor was focused on.
	* @sa FFocusInfo
	* @returns true if the gaze rays hit any actor, otherwise false.
	*/
	bool Focus(GazeIndex gazeIndex, float maxDistance, float radius, APlayerCameraManager* playerCamera, TEnumAsByte<ECollisionChannel> TraceChannel, FFocusInfo &focusInfo, FVector &gazeOrigin, FVector &gazeTarget);
	/** Get the combined gaze ray and focusInfo when any candidate object was focused on. */
	bool Focus(GazeIndex gazeIndex, float maxDistance, APlayerCameraManager* playerCamera, TEnumAsByte<ECollisionChannel> TraceChannel, FFocusInfo &focusInfo, FVector &gazeOrigin, FVector &gazeTarget);
	/** Get the combined gaze ray and focusInfo when any candidate object was focused on. */
	bool Focus(GazeIndex gazeIndex, APlayerCameraManager* playerCamera, TEnumAsByte<ECollisionChannel> TraceChannel, FFocusInfo &focusInfo, FVector &gazeOrigin, FVector &gazeTarget);

private:
	bool UpdateData();
	FORCEINLINE void CovertToUnrealLocation(FVector &vector);
	FORCEINLINE void ApplyUnrealWorldToMeterScale(FVector &vector);
	FORCEINLINE void CovertToUnrealQuaternion(FQuat& quat);

private:
	static SRanipalEye_Core* SRanipal_EyeModule;
	int LastUpdateFrame, LastUpdateResult;
	ViveSR::anipal::Eye::EyeData EyeData_;
	ViveSR::anipal::Eye::EyeData_v2 EyeData_v2;

	TMap<EyeShape, float> Weightings;
};
