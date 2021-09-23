// ========= Copyright 2019, HTC Corporation. All rights reserved. ===========
#include <CoreMinimal.h>
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/Actor.h"


#ifdef SRANIPAL_EXPORTS
#define SR_ANIPAL __declspec(dllexport)
#else
#define SR_ANIPAL __declspec(dllimport)
#endif

#pragma once
namespace ViveSR {
	/**
	* Animation pal
	*/
	namespace anipal {

		enum AnipalStatus {
			ERROR	= 0,
			IDLE	= 1,
			WORKING = 2,
		};
	}
}

