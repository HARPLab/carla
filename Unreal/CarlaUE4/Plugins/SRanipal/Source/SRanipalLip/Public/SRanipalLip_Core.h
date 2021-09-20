//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========

#pragma once

#include "CoreMinimal.h"
#include "ViveSR_Enums.h"
#include "SRanipal_Lip_Enums.h"
#include "Engine/Texture2D.h"
/**
 * 
 */

 /** The prediction result relative blend shape.*/
UENUM(BlueprintType)
enum class LipShapeEnum :uint8
{
    Jaw_Forward = 0             UMETA(DisplayName = "Jaw_Forward"),
    Jaw_Right = 1               UMETA(DisplayName = "Jaw_Right"),
    Jaw_Left = 2                UMETA(DisplayName = "Jaw_Left"),
    Jaw_Open = 3                UMETA(DisplayName = "Jaw_Open"),
    Mouth_Ape_Shape = 4         UMETA(DisplayName = "Mouth_Ape_Shape"),
    Mouth_O_Shape = 5           UMETA(DisplayName = "Mouth_O_Shape"),
    Mouth_Pout = 6              UMETA(DisplayName = "Mouth_Pout"),
    Mouth_Lower_Right = 7       UMETA(DisplayName = "Mouth_Lower_Right"),
    Mouth_Lower_Left = 8        UMETA(DisplayName = "Mouth_Lower_Left"),
    Mouth_Smile_Right = 9       UMETA(DisplayName = "Mouth_Smile_Right"),
    Mouth_Smile_Left = 10       UMETA(DisplayName = "Mouth_Smile_Left"),
    Mouth_Sad_Right = 11        UMETA(DisplayName = "Mouth_Sad_Right"),
    Mouth_Sad_Left = 12         UMETA(DisplayName = "Mouth_Sad_Left"),
    Cheek_Puff_Right = 13       UMETA(DisplayName = "Cheek_Puff_Right"),
    Cheek_Puff_Left = 14        UMETA(DisplayName = "Cheek_Puff_Left"),
    Mouth_Lower_Inside = 15     UMETA(DisplayName = "Mouth_Lower_Inside"),
    Mouth_Upper_Inside = 16     UMETA(DisplayName = "Mouth_Upper_Inside"),
    Mouth_Lower_Overlay = 17    UMETA(DisplayName = "Mouth_Lower_Overlay"),
    Mouth_Upper_Overlay = 18    UMETA(DisplayName = "Mouth_Upper_Overlay"),
    Cheek_Suck = 19             UMETA(DisplayName = "Cheek_Suck"),
    Mouth_LowerRight_Down = 20  UMETA(DisplayName = "Mouth_LowerRight_Down"),
    Mouth_LowerLeft_Down = 21   UMETA(DisplayName = "Mouth_LowerLeft_Down"),
    Mouth_UpperRight_Up = 22    UMETA(DisplayName = "Mouth_UpperRight_Up"),
    Mouth_UpperLeft_Up = 23     UMETA(DisplayName = "Mouth_UpperLeft_Up"),
    Mouth_Philtrum_Right = 24   UMETA(DisplayName = "Mouth_Philtrum_Right"),
    Mouth_Philtrum_Left = 25    UMETA(DisplayName = "Mouth_Philtrum_Left"),
    Max = 26                    UMETA(Hidden),
    None = 63                   UMETA(DisplayName = "None"),
};

/** The prediction result relative blend shape.*/
UENUM(BlueprintType)
enum class LipShapeEnum_v2 :uint8
{
    Jaw_Right = 0                UMETA(DisplayName = "Jaw_Right"),
    Jaw_Left = 1                 UMETA(DisplayName = "Jaw_Left"),
    Jaw_Forward = 2              UMETA(DisplayName = "Jaw_Forward"),
    Jaw_Open = 3                 UMETA(DisplayName = "Jaw_Open"),
    Mouth_Ape_Shape = 4          UMETA(DisplayName = "Mouth_Ape_Shape"),
    Mouth_Upper_Right = 5        UMETA(DisplayName = "Mouth_Upper_Right"),
    Mouth_Upper_Left = 6         UMETA(DisplayName = "Mouth_Upper_Left"),
    Mouth_Lower_Right = 7        UMETA(DisplayName = "Mouth_Lower_Right"),
    Mouth_Lower_Left = 8         UMETA(DisplayName = "Mouth_Lower_Left"),
    Mouth_Upper_Overturn = 9     UMETA(DisplayName = "Mouth_Upper_Overturn"),
    Mouth_Lower_Overturn = 10    UMETA(DisplayName = "Mouth_Lower_Overturn"),
    Mouth_Pout = 11              UMETA(DisplayName = "Mouth_Pout"),
    Mouth_Smile_Right = 12       UMETA(DisplayName = "Mouth_Smile_Right"),
    Mouth_Smile_Left = 13        UMETA(DisplayName = "Mouth_Smile_Left"),
    Mouth_Sad_Right = 14         UMETA(DisplayName = "Mouth_Sad_Right"),
    Mouth_Sad_Left = 15          UMETA(DisplayName = "Mouth_Sad_Left"),
    Cheek_Puff_Right = 16        UMETA(DisplayName = "Cheek_Puff_Right"),
    Cheek_Puff_Left = 17         UMETA(DisplayName = "Cheek_Puff_Left"),
    Cheek_Suck = 18              UMETA(DisplayName = "Cheek_Suck"),
    Mouth_Upper_UpRight = 19     UMETA(DisplayName = "Mouth_Upper_UpRight"),
    Mouth_Upper_UpLeft = 20      UMETA(DisplayName = "Mouth_Upper_UpLeft"),
    Mouth_Lower_DownRight = 21   UMETA(DisplayName = "Mouth_Lower_DownRight"),
    Mouth_Lower_DownLeft = 22    UMETA(DisplayName = "Mouth_Lower_DownLeft"),
    Mouth_Upper_Inside = 23      UMETA(DisplayName = "Mouth_Upper_Inside"),
    Mouth_Lower_Inside = 24      UMETA(DisplayName = "Mouth_Lower_Inside"),
    Mouth_Lower_Overlay = 25     UMETA(DisplayName = "Mouth_Lower_Overlay"),
    Tongue_LongStep1 = 26        UMETA(DisplayName = "Tongue_LongStep1"),
    Tongue_Left = 27             UMETA(DisplayName = "Tongue_Left"),
    Tongue_Right = 28            UMETA(DisplayName = "Tongue_Right"),
    Tongue_Up = 29               UMETA(DisplayName = "Tongue_Up"),
    Tongue_Down = 30             UMETA(DisplayName = "Tongue_Down"),
    Tongue_Roll = 31             UMETA(DisplayName = "Tongue_Roll"),
    Tongue_LongStep2 = 32        UMETA(DisplayName = "Tongue_LongStep2"),
    Tongue_UpRight_Morph = 33    UMETA(DisplayName = "Tongue_UpRight_Morph"),
    Tongue_UpLeft_Morph = 34     UMETA(DisplayName = "Tongue_UpLeft_Morph"),
    Tongue_DownRight_Morph = 35  UMETA(DisplayName = "Tongue_DownRight_Morph"),
    Tongue_DownLeft_Morph = 36   UMETA(DisplayName = "Tongue_DownLeft_Morph"),
    Max = 37                     UMETA(Hidden),
    None = 63                    UMETA(DisplayName = "None"),
};


/**
 * @brief The internal static class about lip relative API
 */
class SRANIPALLIP_API SRanipalLip_Core
{
public:
	SRanipalLip_Core();
	~SRanipalLip_Core();

	static SRanipalLip_Core* Instance();
	static void DestroyLipModule();

	/**
	* Get the LipData from Lip engine. When use this API, you should assign an uint8 image buffer(size = 800*400) to data.image.
	* @return one of ViveSR.Error
	*/
	int GetLipData_(ViveSR::anipal::Lip::LipData *data);
	/**
	* Get the LipData from Lip engine. When use this API, you should assign an uint8 image buffer(size = 800*400) to data.image.
	* @return one of ViveSR.Error
	*/
	int GetLipData_v2(ViveSR::anipal::Lip::LipData_v2 *data);

	/**
	* Get a dictionary that stores individual weighting with respect to LipShape
	* @param shapes A dictionary that stores individual weighting with respect to LipShape.
	* @return true if there is any new data.
	*/
	bool GetLipWeightings(TMap<LipShapeEnum, float> &shapes);
	/**
	* Get a dictionary that stores individual weighting with respect to LipShape
	* @param shapes A dictionary that stores individual weighting with respect to LipShape.
	* @return true if there is any new data.
	*/
	bool GetLipWeightings_v2(TMap<LipShapeEnum_v2, float> &shapes);

	/**
	* Update given texture2D with lip image.
	* @param texture A texture2D sholud be defined as UTexture2D::CreateTransient(ImageWidth, ImageHeight, PF_A8)
	* @return true if there is any new data.
	*/
	bool UpdateTexture(UTexture2D *&texture);

public:
	int ImageWidth, ImageHeight;
	int SingleImageWidth;

private:
	bool UpdateData();

private:
	static SRanipalLip_Core* SRanipal_LipModule;
	int LastUpdateFrame, LastUpdateResult;
	ViveSR::anipal::Lip::LipData lipData;
	ViveSR::anipal::Lip::LipData_v2 lipData_v2;
	FUpdateTextureRegion2D* updateRegion;
	uint8* LipImageBuffer;
	TMap<LipShapeEnum, float> Weightings;
	TMap<LipShapeEnum_v2, float> Weightings_v2;
};
