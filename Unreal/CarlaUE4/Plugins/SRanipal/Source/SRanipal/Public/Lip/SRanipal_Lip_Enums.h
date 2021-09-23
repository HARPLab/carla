// ========= Copyright 2018, HTC Corporation. All rights reserved. ===========

#pragma once
#include "CoreUObject/Public/UObject/Class.h"
#include "Core.h"

/** Vive Super Reality */
namespace ViveSR
{
	/** Animation pal */
	namespace anipal
	{
		/** Lip module */
		namespace Lip
		{
			const int blend_shape_nums = 60;
			/**
			* @struct PredictionData
			* A struct containing all data listed below.
			*/
			struct PredictionData
			{
				/** The prediction result listing all of @ref LipShape in [0,1].*/
				float blend_shape_weight[27];	
			};

			/**
			* @struct LipData
			* A struct containing all data listed below.
			*/
			struct LipData
			{
				/** The frame sequence.*/
				int frame_sequence;
				/** The time when the frame was capturing. in millisecond. */
				int timestamp;
				/** The raw buffer. width=800, height=400, channel=1 */
				unsigned char *image;
				/** The prediction result listing all of @ref LipShape in [0,1]. */
				PredictionData prediction_data;
			};

			/**
			* @struct PredictionData
			* A struct containing all data listed below.
			*/
			struct PredictionData_v2
			{
				float blend_shape_weight[blend_shape_nums];	/*!<The prediction result listing all of @ref LipShape in [0,1].*/
			};
			/**
			* @struct LipData
			* A struct containing all data listed below.
			*/
			struct LipData_v2
			{
				int frame_sequence;				/*!<The frame sequence.*/
				int timestamp;					/*!<The time when the frame was capturing. in millisecond.*/
				unsigned char *image;					/*!<The raw buffer. width=800, height=400, channel=1*/
				PredictionData_v2 prediction_data;	/*!<The prediction result listing all of @ref LipShape in [0,1].*/
			};
		}
	}
}
UENUM(BlueprintType)
enum class SupportedLipVersion : uint8
{
	version1,
	version2,
};