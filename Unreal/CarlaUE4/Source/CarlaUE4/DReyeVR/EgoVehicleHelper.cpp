#include "EgoVehicleHelper.h"
#include "EgoVehicle.h"
#include "Carla/Vehicle/VehicleControl.h"      // FVehicleControl
#include "DrawDebugHelpers.h"                  // Debug Line/Sphere
#include "Engine/World.h"                      // GetWorld
#include "GameFramework/Actor.h"               // Destroy
#include "HeadMountedDisplayFunctionLibrary.h" // SetTrackingOrigin, GetWorldToMetersScale
#include "HeadMountedDisplayTypes.h"           // ESpectatorScreenMode
#include "Kismet/GameplayStatics.h"            // GetPlayerController
#include "Kismet/KismetSystemLibrary.h"        // PrintString, QuitGame
#include "Math/Rotator.h"                      // RotateVector, Clamp
#include "Math/UnrealMathUtility.h"            // Clamp
#include "Components/SphereComponent.h"		   // Sphere Starter Content

#include <algorithm>



std::tuple<float, float> AEgoVehicleHelpers::GetAngles(FVector UnitGazeVec, FVector RotVec)

{
	// Normalize input vectors
	UnitGazeVec = UnitGazeVec / UnitGazeVec.Size();
	RotVec = RotVec / RotVec.Size();

	// Rotating Vectors back to world coordinate frame to get angles pitch and yaw
	float Unit_x0 = UnitGazeVec[0];
	float Unit_y0 = UnitGazeVec[1];
	float Unit_z0 = UnitGazeVec[2];
	float Rot_x0 = RotVec[0];
	float Rot_y0 = RotVec[1];
	float Rot_z0 = RotVec[2];

	// Multiplying UnitGazeVec by Rotation Z Matrix
	float UnitGaze_yaw = atan2(Unit_y0, Unit_x0); // rotation about z axis
	float Unit_x1 = Unit_x0 * cos(UnitGaze_yaw) + Unit_y0 * sin(UnitGaze_yaw);
	float Unit_y1 = -Unit_x0 * sin(UnitGaze_yaw) + Unit_y0 * cos(UnitGaze_yaw);
	float Unit_z1 = Unit_z0;

	// Multiplying UnitGazeVec_1 by Rotation Y Matrix
	float UnitGaze_pitch = atan2(Unit_z1, Unit_x1); // rotation about y axis
	float Unit_x2 = Unit_x1 * cos(UnitGaze_pitch) + Unit_z1 * sin(UnitGaze_pitch);
	float Unit_y2 = Unit_y1;
	float Unit_z2 = -Unit_x1 * sin(UnitGaze_pitch) + Unit_z1 * cos(UnitGaze_pitch);

	//UE_LOG(LogTemp, Log, TEXT("x2 logging %f"), Unit_x2);
	//UE_LOG(LogTemp, Log, TEXT("y2 logging %f"), Unit_y2);
	//UE_LOG(LogTemp, Log, TEXT("z2 logging %f"), Unit_z2);

	// Multiplying RotVec by Rotation Z Matrix
	float Rot_x1 = Rot_x0 * cos(UnitGaze_yaw) + Rot_y0 * sin(UnitGaze_yaw);
	float Rot_y1 = -Rot_x0 * sin(UnitGaze_yaw) + Rot_y0 * cos(UnitGaze_yaw);
	float Rot_z1 = Rot_z0;

	// Multiplying RotVec_1 by Rotation Y Matrix
	float Rot_x2 = Rot_x1 * cos(UnitGaze_pitch) + Rot_z1 * sin(UnitGaze_pitch);
	float Rot_y2 = Rot_y1;
	float Rot_z2 = -Rot_x1 * sin(UnitGaze_pitch) + Rot_z1 * cos(UnitGaze_pitch);

	// Get Yaw
	float yaw = atan2(Rot_y2, Rot_x2);
	//UE_LOG(LogTemp, Log, TEXT("yaw log: %f"), yaw);
	//VehicleInputs.yaw = yaw;
	

	// Get Pitch
	float pitch = atan2(Rot_z2, Rot_x2);
	//UE_LOG(LogTemp, Log, TEXT("pitch log: %f"), pitch);
	//VehicleInputs.pitch = pitch;

	return { pitch, yaw };
}

/*
void AEgoVehicle::PeripheralResponseButtonPressed()
{
	UE_LOG(LogTemp, Log, TEXT("Hit"));
	VehicleInputs.ButtonPressed = true;
}

void AEgoVehicle::PeripheralResponseButtonReleased()
{
	UE_LOG(LogTemp, Log, TEXT("Hit"));
	VehicleInputs.ButtonPressed = false;
}
*/

FVector AEgoVehicleHelpers::GenerateRotVec(FVector UnitGazeVec, float yawMaxIn, float pitchMaxIn, float vert_offsetIn)
{
	// Normalize input vector
	UnitGazeVec = UnitGazeVec / UnitGazeVec.Size();
	float Unit_x0 = UnitGazeVec[0];
	float Unit_y0 = UnitGazeVec[1];
	float Unit_z0 = UnitGazeVec[2];

	float UnitGaze_yaw = atan2(Unit_y0, Unit_x0); // rotation about z axis
	float Unit_x1 = Unit_x0 * cos(UnitGaze_yaw) + Unit_y0 * sin(UnitGaze_yaw);
	float Unit_y1 = -Unit_x0 * sin(UnitGaze_yaw) + Unit_y0 * cos(UnitGaze_yaw);
	float Unit_z1 = Unit_z0;

	float UnitGaze_pitch = atan2(Unit_z1, Unit_x1); // rotation about y axis
	float Unit_x2 = Unit_x1 * cos(UnitGaze_pitch) + Unit_z1 * sin(UnitGaze_pitch);
	float Unit_y2 = Unit_y1;
	float Unit_z2 = -Unit_x1 * sin(UnitGaze_pitch) + Unit_z1 * cos(UnitGaze_pitch);

	// (1, 0, 0) Vector
	//UE_LOG(LogTemp, Log, TEXT("x2 logging %f"), Unit_x2);
	//UE_LOG(LogTemp, Log, TEXT("y2 logging %f"), Unit_y2);
	//UE_LOG(LogTemp, Log, TEXT("z2 logging %f"), Unit_z2);

	float pitch_dist_pos = tan(pitchMaxIn + vert_offsetIn);
	float pitch_dist_neg = tan(-pitchMaxIn + vert_offsetIn);
	float yaw_dist = tan(yawMaxIn);
	//UE_LOG(LogTemp, Log, TEXT("pitch dist, %f"), pitch_dist);
	//UE_LOG(LogTemp, Log, TEXT("yaw dist, %f"), yaw_dist);

	float Rot_x0 = 1.f;
	float Rot_y0 = FMath::RandRange(-yaw_dist, yaw_dist);
	float Rot_z0 = FMath::RandRange(pitch_dist_neg, pitch_dist_pos);
	//UE_LOG(LogTemp, Log, TEXT("Rot_y0, %f"), Rot_y0);
	//UE_LOG(LogTemp, Log, TEXT("Rot_z0, %f"), Rot_z0);

	// Inverse rotation matrix to get back to world coordinates
	float Rot_x1 = Rot_x0 * cos(UnitGaze_pitch) - Rot_z0 * sin(UnitGaze_pitch);
	float Rot_y1 = Rot_y0;
	float Rot_z1 = Rot_x0 * sin(UnitGaze_pitch) + Rot_z0 * cos(UnitGaze_pitch);

	float Rot_x2 = Rot_x1 * cos(UnitGaze_yaw) - Rot_y1 * sin(UnitGaze_yaw);
	float Rot_y2 = Rot_x1 * sin(UnitGaze_yaw) + Rot_y1 * cos(UnitGaze_yaw);
	float Rot_z2 = Rot_z1;

	FVector RotVec = FVector(Rot_x2, Rot_y2, Rot_z2);
	// normalize
	RotVec = RotVec / RotVec.Size();

	return RotVec;
}

FVector AEgoVehicleHelpers::GenerateRotVecGivenAngles(FVector UnitGazeVec, float yaw, float pitch)
{
	// Normalize input vector
	UnitGazeVec = UnitGazeVec / UnitGazeVec.Size();
	float Unit_x0 = UnitGazeVec[0];
	float Unit_y0 = UnitGazeVec[1];
	float Unit_z0 = UnitGazeVec[2];

	float UnitGaze_yaw = atan2(Unit_y0, Unit_x0); // unit yaw
	float Unit_x1 = Unit_x0 * cos(UnitGaze_yaw) + Unit_y0 * sin(UnitGaze_yaw);
	float Unit_y1 = -Unit_x0 * sin(UnitGaze_yaw) + Unit_y0 * cos(UnitGaze_yaw);
	float Unit_z1 = Unit_z0;

	float UnitGaze_pitch = atan2(Unit_z1, Unit_x1); // unit pitch

	float Rot_x0 = 1.f;
	float Rot_y0 = tan(yaw);
	float Rot_z0 = tan(pitch);

	// Inverse rotation matrix to get back to world coordinates
	float Rot_x1 = Rot_x0 * cos(UnitGaze_pitch) - Rot_z0 * sin(UnitGaze_pitch);
	float Rot_y1 = Rot_y0;
	float Rot_z1 = Rot_x0 * sin(UnitGaze_pitch) + Rot_z0 * cos(UnitGaze_pitch);

	float Rot_x2 = Rot_x1 * cos(UnitGaze_yaw) - Rot_y1 * sin(UnitGaze_yaw);
	float Rot_y2 = Rot_x1 * sin(UnitGaze_yaw) + Rot_y1 * cos(UnitGaze_yaw);
	float Rot_z2 = Rot_z1;

	FVector RotVec = FVector(Rot_x2, Rot_y2, Rot_z2);
	RotVec = RotVec / RotVec.Size();

	return RotVec;
}

GazeDataEntry AEgoVehicleHelpers::SensorData2GazeDataEntry(const DReyeVR::SensorData* SensorDataS)
{
    double timestamp, x, y, confidence;

    timestamp = SensorDataS->TimestampCarla;

    // get the pitch and yaw from the 3D vectors because IBDT works with 2D data
    // (prefer pitch/yaw vs pixel coords so there is no off-foveal distance distortion)

    // this vector is relative to the neutral (1,0,0) head gaze
    auto ComboEyeGazeVector = SensorDataS->Combined.GazeRay;
    auto gaze_angles_tuple = GetAngles(FVector(1,0,0), ComboEyeGazeVector);
    x = std::get<0>(gaze_angles_tuple); // head2gaze_pitch
    y = std::get<1>(gaze_angles_tuple); // head2gaze_yaw

    // confidence is low if x, y is 0, 0 -- weird bug in VIVE tracker
    if (x==0 && y==0)
        confidence=0;
    else
        confidence=1;

    return GazeDataEntry(timestamp, confidence, x, y);
}
