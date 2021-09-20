///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//                           License Agreement
//                     For Vive Super Reality Library
//
// Copyright (c) 2019,		HTC Corporation
//
// All rights reserved. Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//

#pragma once
#include "Core.h"
#include "CoreUObject/Public/UObject/Class.h"
#include <stdint.h>
#include "SRanipal_Eyes_Enums.generated.h"

namespace ViveSR {
	namespace anipal {
		/**
		* Eye module
		*/
		namespace Eye
		{
#pragma region bit_mask_enum
			/** @enum SingleEyeDataValidity
			An enum type for getting validity from the structure: eye data's bitmask
			*/
			enum SingleEyeDataValidity {
				/** The validity of the origin of gaze of the eye data */
				SINGLE_EYE_DATA_GAZE_ORIGIN_VALIDITY,
				/** The validity of the direction of gaze of the eye data */
				SINGLE_EYE_DATA_GAZE_DIRECTION_VALIDITY,
				/** The validity of the diameter of gaze of the eye data */
				SINGLE_EYE_DATA_PUPIL_DIAMETER_VALIDITY,
				/** The validity of the openness of the eye data */
				SINGLE_EYE_DATA_EYE_OPENNESS_VALIDITY,
				/** The validity of normalized position of pupil */
				SINGLE_EYE_DATA_PUPIL_POSITION_IN_SENSOR_AREA_VALIDITY
			};

			enum TrackingImprovement
			{
				TRACKING_IMPROVEMENT_USER_POSITION_HMD,
				TRACKING_IMPROVEMENT_CALIBRATION_CONTAINS_POOR_DATA,
				TRACKING_IMPROVEMENT_CALIBRATION_DIFFERENT_BRIGHTNESS,
				TRACKING_IMPROVEMENT_IMAGE_QUALITY,
				TRACKING_IMPROVEMENT_INCREASE_EYE_RELIEF,
			};
#pragma endregion
			/** @struct SingleEyeData
			* A struct containing status related an eye.
			* @image html EyeData.png width=1040px height=880px
			*/
			typedef struct SingleEyeData
			{
				/** The bits containing all validity for this frame.*/
				uint64_t eye_data_validata_bit_mask;
				/** The point in the eye from which the gaze ray originates in millimeters.(right-handed coordinate system)*/
				FVector gaze_origin_mm;
				/** The normalized gaze direction of the eye in [0,1].(right-handed coordinate system)*/
				FVector gaze_direction_normalized;
				/** The diameter of the pupil in millimeters.*/
				float pupil_diameter_mm;
				/** A value representing how open the eye is clamped between 0(fully closed) and 1(fully open).*/
				float eye_openness;
				/** The normalized position of a pupil in [0,1]*/
				FVector2D pupil_position_in_sensor_area;

				bool GetValidity(SingleEyeDataValidity validity) {
					return (eye_data_validata_bit_mask & (uint64)validity) > 0;
				}

			}SingleEyeData;

			typedef struct CombinedEyeData
			{
				SingleEyeData eye_data;
				bool convergence_distance_validity;
				float convergence_distance_mm;
			};

			struct TrackingImprovements
			{
				int count;
				union
				{
					struct
					{
						TrackingImprovement t0;
						TrackingImprovement t1;
						TrackingImprovement t2;
						TrackingImprovement t3;
						TrackingImprovement t4;
						TrackingImprovement t5;
						TrackingImprovement t6;
						TrackingImprovement t7;
						TrackingImprovement t8;
						TrackingImprovement t9;
					};
					TrackingImprovement elem_[10];
				};
			};

			/** @struct VerboseData
			* A struct containing all data listed below.
			*/
			typedef struct VerboseData
			{
				/** A instance of the struct as @ref EyeData related to the left eye*/
				SingleEyeData left;
				/** A instance of the struct as @ref EyeData related to the right eye*/
				SingleEyeData right;
				/** A instance of the struct as @ref EyeData related to the combined eye*/
				CombinedEyeData combined;
				TrackingImprovements tracking_improvements;
			}VerboseData;

			/** @struct EyeData
			* A struct containing all data listed below.
			*/
			struct EyeData
			{
				bool no_user;
				/** The frame sequence.*/
				int frame_sequence;
				/** The time when the frame was capturing. in millisecond.*/
				int timestamp;
				VerboseData verbose_data;
			};
			struct SingleEyeExpression
			{
				float eye_wide; /*!<A value representing how open eye widely.*/
				float eye_squeeze; /*!<A value representing how the eye is closed tightly.*/
				float eye_frown; /*!<A value representing user's frown.*/
			};
			struct EyeExpression
			{
				SingleEyeExpression left;
				SingleEyeExpression right;
			};
			/** @struct EyeData
			* A struct containing all data listed below.
			*/
			struct EyeData_v2
			{
				/** Indicate if there is a user in front of HMD. */
				bool no_user;
				/** The frame sequence.*/
				int frame_sequence;
				/** The time when the frame was capturing. in millisecond.*/
				int timestamp;
				VerboseData verbose_data;
				EyeExpression expression_data;
			};
			/** @struct GazeRayParameter
			* A struct containing all data listed below.
			*/
			struct GazeRayParameter {
				/** The sensitive factor of gaze ray in [0,1]. The bigger factor is, the more sensitive the gaze ray is.*/
				double sensitive_factor;
			};

			/** @struct EyeParameter
			* A struct containing all data listed below.
			*/
			struct EyeParameter {
				GazeRayParameter gaze_ray_parameter;
			};
		}
	}
}
UENUM(BlueprintType)
enum class SupportedEyeVersion : uint8
{
	version1,
	version2,
};
/** Used to specify eye's index. */
UENUM(BlueprintType)
enum class EyeIndex :uint8 {
	LEFT		UMETA(DisplayName = "LEFT"),
	RIGHT		UMETA(DisplayName = "RIGHT"),
	MAX			UMETA(Hidden),
};

/** Used to specify gaze ray's index. */
UENUM(BlueprintType)
enum class GazeIndex :uint8 {
	LEFT		UMETA(DisplayName = "LEFT"),
	RIGHT		UMETA(DisplayName = "RIGHT"),
	COMBINE		UMETA(DisplayName = "COMBINE"),
};

/** The avatar's eye relative blend shape.*/
UENUM(BlueprintType)
enum class EyeShape :uint8 {
	Eye_Left_Blink = 0	 UMETA(DisplayName = "Eye_Left_Blink"),
	Eye_Left_Wide = 1	 UMETA(DisplayName = "Eye_Left_Wide"),
	Eye_Left_Right = 2	 UMETA(DisplayName = "Eye_Left_Right"),
	Eye_Left_Left = 3	 UMETA(DisplayName = "Eye_Left_Left"),
	Eye_Left_Up = 4		 UMETA(DisplayName = "Eye_Left_Up"),
	Eye_Left_Down = 5	 UMETA(DisplayName = "Eye_Left_Down"),
	Eye_Right_Blink = 6	 UMETA(DisplayName = "Eye_Right_Blink"),
	Eye_Right_Wide = 7	 UMETA(DisplayName = "Eye_Right_Wide"),
	Eye_Right_Right = 8	 UMETA(DisplayName = "Eye_Right_Right"),
	Eye_Right_Left = 9	 UMETA(DisplayName = "Eye_Right_Left"),
	Eye_Right_Up = 10	 UMETA(DisplayName = "Eye_Right_Up"),
	Eye_Right_Down = 11  UMETA(DisplayName = "Eye_Right_Down"),
	Eye_Frown = 12		 UMETA(DisplayName = "Eye_Frown"),
    Eye_Left_Squeeze = 13  UMETA(DisplayName = "Eye_Left_Squeeze"),
    Eye_Right_Squeeze = 14 UMETA(DisplayName = "Eye_Right_Squeeze"),
    Max = 15			   UMETA(Hidden),
    None = 63			   UMETA(DisplayName = "None"),
};

UENUM(BlueprintType)
enum class CalibrationResult :uint8
{
	SUCCESS UMETA(DisplayName = "SUCCESS"),
	FAIL UMETA(DisplayName = "FAIL"),
	BUSY UMETA(DisplayName = "BUSY"),
};

/** A struct containing all focus information listed below. */
USTRUCT(BlueprintType)
struct FFocusInfo
{
	GENERATED_BODY()

	/** Pointer to the actor which been looking at*/
	UPROPERTY(BlueprintReadWrite, Category = "SRanipal|Struct")
		TWeakObjectPtr<AActor> actor;
	/** The hit position in world space. */
	UPROPERTY(BlueprintReadWrite, Category = "SRanipal|Struct")
		FVector point;
	/** The hit normal in world space. */
	UPROPERTY(BlueprintReadWrite, Category = "SRanipal|Struct")
		FVector normal;
	/** The hit distance from gazeOrigin to gazeTarget */
	UPROPERTY(BlueprintReadWrite, Category = "SRanipal|Struct")
		float distance;
};