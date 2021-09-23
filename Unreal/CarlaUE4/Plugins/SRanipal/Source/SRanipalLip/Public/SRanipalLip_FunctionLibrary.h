//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SRanipalLip_Core.h"
#include "SRanipalLip_FunctionLibrary.generated.h"

/**
 * @brief SRanipal function library about lip function.
 */
UCLASS()
class SRANIPALLIP_API USRanipalLip_FunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	* @summary Start specific version of SRanipal lip framework. 
	* This function blueprint will NOT work if the "Enable Lip by Default" is enabled in project settings.
	*/
	UFUNCTION(BlueprintCallable, Category = "SRanipal|Lip")
		static void StartLipFramework(SupportedLipVersion version);

	/**
	* @summary Stop SRanipal lip framework.
	* This function blueprint will NOT work if the "Enable Lip by Default" is enabled in project settings.
	*/
	UFUNCTION(BlueprintCallable, Category = "SRanipal|Lip")
		static void StopLipFramework();

	/**
	* @summary Get a TArray that stores weighting with respect to LipShape.
	* @param[out] weightings A TArray that stores individual weighting with respect to LipShape
	*/
	UFUNCTION(BlueprintCallable, Category = "SRanipal|Lip")
		static void GetLipWeightings(TMap<LipShapeEnum, float> &shapes);

	/**
	* @summary Get a TArray that stores weighting with respect to LipShape.
	* @param[out] weightings A TArray that stores individual weighting with respect to LipShape
	*/
	UFUNCTION(BlueprintCallable, Category = "SRanipal|Lip")
		static void GetLipWeightings_v2(TMap<LipShapeEnum_v2, float> &shapes);

	/**
	* @summary Update given texture2D with lip image.
	* @param[out] texture Target texture.
	*/
	UFUNCTION(BlueprintCallable, Category = "SRanipal|Lip")
		static void UpdateTexture(UTexture2D *&texture);

};
