#pragma once

#include "Camera/CameraComponent.h"            // UCameraComponent
#include "Carla/Vehicle/CarlaWheeledVehicle.h" // Steering, Throttle, Brake, etc.
#include "Components/AudioComponent.h"         // UAudioComponent
#include "Components/InputComponent.h"         // InputComponent
#include "Components/SceneComponent.h"         // USceneComponent
#include "CoreMinimal.h"                       // Unreal functions
#include "DReyeVRHUD.h"                        // ADReyeVRHUD
#include "EyeTracker.h"                        // AEyeTracker
#include "ImageUtils.h"                        // CreateTexture2D
#include "WheeledVehicle.h"                    // VehicleMovementComponent
#include "LightBall.h"						   // ALightBall
#include <stdio.h>
#include <vector>
#include <tuple>

#include "GazeData.h"
#include "Carla/Sensor/DReyeVRSensorData.h"

// change into namespace

namespace AEgoVehicleHelpers
{
	DReyeVR::UserInputs VehicleInputs;   // struct for user inputs

	std::tuple<float, float> GetAngles(FVector UnitGazeVec, FVector RotVec);
	//void PeripheralResponseButtonPressed();
	//void PeripheralResponseButtonReleased();
	FVector GenerateRotVec(FVector UnitGazeVec, float yawMax, float pitchMax, float vert_offset);
	FVector GenerateRotVecGivenAngles(FVector UnitGazeVec, float yaw, float pitch);
    GazeDataEntry SensorData2GazeDataEntry(const DReyeVR::SensorData* SensorDataS);
};