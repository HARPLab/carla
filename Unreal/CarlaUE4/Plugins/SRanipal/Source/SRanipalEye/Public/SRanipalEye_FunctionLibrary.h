//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SRanipal_Eyes_Enums.h"
#include "Camera/PlayerCameraManager.h"
#include "SRanipalEye_FunctionLibrary.generated.h"

/**
 * @brief SRanipal blueprint function library about eye function.
 */
UCLASS()
class SRANIPALEYE_API USRanipalEye_FunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	* @summary Start specific version of SRanipal eye framework.
	* This function blueprint will NOT work if the "Enable Eye by Default" is enabled in project settings.
	*/
	UFUNCTION(BlueprintCallable, Category = "SRanipal|Eye")
		static void StartEyeFramework(SupportedEyeVersion version);

	/**
	* @summary Stop specific version of SRanipal eye framework.
	* This function blueprint will NOT work if the "Enable Eye by Default" is enabled in project settings.
	*/
	UFUNCTION(BlueprintCallable, Category = "SRanipal|Eye")
		static void StopEyeFramework();

	/**
	 * Get the openness of specific eye.
	 * @param eye Specific which eye.
	 * @param[out] openness The eye openness is clamped to the range [0,1]. when openness = 0, eye is closed. when openness = 1 eye is open.
	 * @return true if the openess is valid, otherwise false.
	 */
	UFUNCTION(BlueprintCallable, Category = "SRanipal|Eye")
		static bool GetEyeOpenness(EyeIndex eye, float &openness);

	/**
	 * Get the position of specific pupil. When position = (1, 1), eye looking up and right. When position = (-1, -1), looking down and left.
	 * @param eye Specific which eye.
	 * @param[out] position The 2d vector specify the pupil's position.
	 * @return true if the position is valid, otherwise false.
	 */
	UFUNCTION(BlueprintCallable, Category = "SRanipal|Eye")
		static bool GetPupilPosition(EyeIndex eye, FVector2D &position);

	/**
	* Get the gaze ray of specific eye.
	* @param param gazeIndex Specific which eye.
	* @param[out] origin The starting point of the ray in local coordinates.
	* @param[out] direction The direction of the ray.
	* @return true if the gaze ray is valid, otherwise false.
	*/
	UFUNCTION(BlueprintCallable, Category = "SRanipal|Eye")
		static bool GetGazeRay(GazeIndex gazeIndex, UPARAM(ref) FVector &origin, UPARAM(ref) FVector &direction);


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
	UFUNCTION(BlueprintCallable, Category = "SRanipal|Eye")
		static bool Focus(GazeIndex gazeIndex, float maxDistance, float radius, APlayerCameraManager* playerCamera, TEnumAsByte<ECollisionChannel> TraceChannel, FFocusInfo &focusInfo, FVector &gazeOrigin, FVector &gazeTarget);

	/**
	* Get a dictionary that stores individual weighting with respect to EyeShape.
	* @param[out] weightings A TArray that stores individual weighting with respect to EyeShape.
	* @return true if there is any new data.
	*/
	UFUNCTION(BlueprintCallable, Category = "SRanipal|Eye")
		static bool GetEyeWeightings(TMap<EyeShape, float> &shapes);
	
};
