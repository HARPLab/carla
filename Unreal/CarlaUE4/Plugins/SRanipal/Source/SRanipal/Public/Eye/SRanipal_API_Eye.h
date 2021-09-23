#pragma once
#include "ViveSR_Enums.h"
#include "SRanipal_Enums.h"
#include "SRanipal_Eyes_Enums.h"

#ifdef SRANIPAL_EXPORTS
#define SR_ANIPAL __declspec(dllexport)
#else
#define SR_ANIPAL __declspec(dllimport)
#endif

extern "C" {
	namespace ViveSR {
		/** Animation pal
		*/
		namespace anipal {
			namespace Eye {
				const int ANIPAL_TYPE_EYE = 0;
				const int ANIPAL_TYPE_EYE_V2 = 2;
				/** Check HMD device is ViveProEye or not.
				@return true : ViveProEye, false : other HMD.
				*/
				SR_ANIPAL bool IsViveProEye();

				/** Gets data from anipal's Eye module.
				* @param data ViveSR::anipal::Eye::EyeData
				* @return Indicates the resulting ViveSR::Error status of this method.
				*/
				SR_ANIPAL int GetEyeData(EyeData *data);

				/** Gets data from anipal's Eye module.
				* @param dat ViveSR::anipal::Eye::EyeData_v2
				* @return Indicates the resulting ViveSR::Error status of this method.
				*/
				SR_ANIPAL int GetEyeData_v2(EyeData_v2 *data);

				/** Sets the parameter of anipal's Eye module.
				* @param parameter ViveSR::anipal::Eye::EyeParameter
				* @return Indicates the resulting ViveSR::Error status of this method.
				*/
				SR_ANIPAL int SetEyeParameter(EyeParameter parameter);

				/** Get the EyeParameter from Eye engine.
				* @param parameter ViveSR::anipal::Eye::EyeParameter
				* @return Indicates the resulting ViveSR::Error status of this method.
				*/
				SR_ANIPAL int GetEyeParameter(EyeParameter *parameter);

				/** Launches anipal's Eye Calibration tool (an overlay program).
				* @param callback (Upcoming feature) A callback method invoked at the end of the calibration process.
				* @return Indicates the resulting ViveSR::Error status of this method.
				*/
				SR_ANIPAL int LaunchEyeCalibration(void* callback);

				/* Synchronization the clock on the device and the clock on the system.
				* @param[in] Trigger for Synchronization function.
				* @return error code. please refer Error in ViveSR_Enums.h
				*/
				SR_ANIPAL int SRanipal_UpdateTimeSync();

				/* Get the system timestamp.
				* @param[out] the value of system timestamp.
				* @return error code. please refer Error in ViveSR_Enums.h
				*/
				SR_ANIPAL int SRanipal_GetSystemTime(int64* time);
			}
		}
	}
}
