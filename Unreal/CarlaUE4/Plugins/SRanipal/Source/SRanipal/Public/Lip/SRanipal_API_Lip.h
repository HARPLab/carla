#pragma once
#include "ViveSR_Enums.h"
#include "SRanipal_Enums.h"
#include "SRanipal_Lip_Enums.h"

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
			namespace Lip {
				const int ANIPAL_TYPE_LIP = 1;
				const int ANIPAL_TYPE_LIP_V2 = 3;
				/** Get the LipData from Lip engine.
				* @param data ViveSR::anipal::Lip::LipData
				* @return one of ViveSR::Error
				*/
				SR_ANIPAL int GetLipData(LipData *data);
				/** Get the LipData from Lip engine.
				* @param data ViveSR::anipal::Lip::LipData_v2
				* @return one of ViveSR::Error
				*/
				SR_ANIPAL int GetLipData_v2(LipData_v2 *data);
			}
		}
	}
}
