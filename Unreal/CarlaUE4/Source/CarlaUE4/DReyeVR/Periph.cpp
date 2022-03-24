#include "EgoSensor.h"

// void AEgoVehicle::GenerateSphere(const FVector &HeadDirection, const FVector &CombinedGazePosn,
//                                  const FRotator &WorldRot, const FVector &CombinedOriginIn, float DeltaTime)
// {
//     float CenterMagnitude = (CombinedGazePosn - CombinedOriginIn).Size();
//     FVector UnitGazeVec = (CombinedGazePosn - CombinedOriginIn) / CenterMagnitude;
//     // UE_LOG(LogTemp, Log, TEXT("UnitGazVec, %s"), *UnitGazeVec.ToString());

//     if (EgoSensor->IsReplaying())
//     {
//         // replay the position (direction) and visibility of the light ball object
//         const DReyeVR::AggregateData *Replay = EgoSensor->GetData();
//         const FVector RotVecDirection = GenerateRotVecGivenAngles(
//             HeadDirection, Replay->GetUserInputs().head2target_yaw, Replay->GetUserInputs().head2target_pitch);
//         LightBallObject->SetLocation(FirstPersonCam->GetComponentLocation() + RotVecDirection *
//         TargetRenderDistance); if (Replay->GetUserInputs().LightOn)
//             LightBallObject->TurnLightOn();
//         else
//             LightBallObject->TurnLightOff();
//         return; // don't generate anything, just exit
//     }

//     // generate stimuli every TimeBetweenFlash second chunks, and log that time
//     // TODO: all these magic numbers need to be parameterized
//     if (TimeSinceIntervalStart < TimeBetweenFlash)
//     {
//         if (TimeSinceIntervalStart == 0.f)
//         {
//             // Generate light posn wrt head direction
//             FVector RotVec = GenerateRotVec(HeadDirection); // random

//             // Get angles between head direction and light posn
//             auto head2light_angles = GetAngles(HeadDirection, RotVec);
//             head2light_pitch = std::get<0>(head2light_angles);
//             head2light_yaw = std::get<1>(head2light_angles);

//             // Generate random time to start flashing during the interval
//             float TimeStartCushion = 0.1f;
//             TimeStart = FMath::RandRange(0.f + TimeStartCushion, TimeBetweenFlash - TimeStartCushion);
//         }
//         else if (FMath::IsNearlyEqual(TimeSinceIntervalStart, TimeStart, 0.05f))
//         {
//             // turn light on

//             LightBallObject->TurnLightOn();
//             light_visible = true;

//             UE_LOG(LogTemp, Log, TEXT("Light On: %d @ %f"), light_visible, TimeSinceIntervalStart);
//         }
//         else if (FMath::IsNearlyEqual(TimeSinceIntervalStart, TimeStart + FlashDuration, 0.05f))
//         {
//             // turn light off
//             LightBallObject->TurnLightOff();
//             light_visible = false;

//             UE_LOG(LogTemp, Log, TEXT("Light Off: %d @ %f"), light_visible, TimeSinceIntervalStart);
//         }
//         TimeSinceIntervalStart += DeltaTime;

//         // update the vehicle inputs so they are persistent between ticks until the TimeSinceIntervalStart/end
//         VehicleInputs.LightOn = light_visible;
//         VehicleInputs.head2target_pitch = head2light_pitch;
//         VehicleInputs.head2target_yaw = head2light_yaw;
//     }
//     else
//     {
//         TimeSinceIntervalStart = 0.f;
//     }

//     // UE_LOG(LogTemp, Log, TEXT("DeltaTime, %f"), DeltaTime);
//     // UE_LOG(LogTemp, Log, TEXT("TimeSinceIntervalStart, %f"), TimeSinceIntervalStart);

//     // Generate the posn of the light given current angles
//     // float TargetRenderDistance = CenterMagnitude*3;
//     // UE_LOG(LogTemp, Log, TEXT("CenterMagnitude, %f"), CenterMagnitude);
//     // RotVec of target from head direction
//     FVector RotVecDirection = GenerateRotVecGivenAngles(HeadDirection, head2light_yaw, head2light_pitch);
//     // UE_LOG(LogTemp, Log, TEXT("RotVec, %s"), *RotVecDirection.ToString());
//     FVector HeadPos = FirstPersonCam->GetComponentLocation();
//     LightBallObject->SetLocation(HeadPos + RotVecDirection * TargetRenderDistance);

//     /*
//     UE_LOG(LogTemp, Log, TEXT("RotVec, %s"), *RotVec.ToString());
//     UE_LOG(LogTemp, Log, TEXT("RotVecDirection, %s"), *RotVecDirection.ToString());
//     */

//     // Calculate gaze angles of light posn wrt eye gaze
//     auto eye2light_angles = GetAngles(UnitGazeVec, RotVecDirection);
//     VehicleInputs.gaze2target_pitch = std::get<0>(eye2light_angles);
//     VehicleInputs.gaze2target_yaw = std::get<1>(eye2light_angles);

//     // Calculate gaze angles of eye gaze wrt head direction
//     /*
//     auto gaze_from_head_angles = GetAngles(HeadDirection, UnitGazeVec);
//     gaze_from_head_pitch = std::get<0>(gaze_from_head_angles);
//     gaze_from_head_yaw = std::get<1>(gaze_from_head_angles);
//     VehicleInputs.gazeHeadPitch = gaze_from_head_pitch;
//     VehicleInputs.gazeHeadYaw = gaze_from_head_yaw;
//     */

//     // Draw debug border markers
//     FVector TopLeft = GenerateRotVecGivenAngles(HeadDirection, -this->yawMax, this->pitchMax + this->vert_offset) *
//                       TargetRenderDistance;
//     FVector TopRight = GenerateRotVecGivenAngles(HeadDirection, this->yawMax, this->pitchMax + this->vert_offset) *
//                        TargetRenderDistance;
//     FVector BotLeft = GenerateRotVecGivenAngles(HeadDirection, -this->yawMax, -this->pitchMax + this->vert_offset) *
//                       TargetRenderDistance;
//     FVector BotRight = GenerateRotVecGivenAngles(HeadDirection, this->yawMax, -this->pitchMax + this->vert_offset) *
//                        TargetRenderDistance;
//     DrawDebugSphere(World, CombinedOriginIn + TopLeft, 4.0f, 12, FColor::Blue);
//     DrawDebugSphere(World, CombinedOriginIn + TopRight, 4.0f, 12, FColor::Blue);
//     DrawDebugSphere(World, CombinedOriginIn + BotLeft, 4.0f, 12, FColor::Blue);
//     DrawDebugSphere(World, CombinedOriginIn + BotRight, 4.0f, 12, FColor::Blue);
// }

// std::tuple<float, float> AEgoVehicle::GetAngles(FVector UnitGazeVec, FVector RotVec) const
// {
//     // Normalize input vectors
//     UnitGazeVec = UnitGazeVec / UnitGazeVec.Size();
//     RotVec = RotVec / RotVec.Size();

//     // Rotating Vectors back to world coordinate frame to get angles pitch and yaw
//     float Unit_x0 = UnitGazeVec[0];
//     float Unit_y0 = UnitGazeVec[1];
//     float Unit_z0 = UnitGazeVec[2];
//     float Rot_x0 = RotVec[0];
//     float Rot_y0 = RotVec[1];
//     float Rot_z0 = RotVec[2];

//     // Multiplying UnitGazeVec by Rotation Z Matrix
//     float UnitGaze_yaw = atan2(Unit_y0, Unit_x0); // rotation about z axis
//     float Unit_x1 = Unit_x0 * cos(UnitGaze_yaw) + Unit_y0 * sin(UnitGaze_yaw);
//     float Unit_y1 = -Unit_x0 * sin(UnitGaze_yaw) + Unit_y0 * cos(UnitGaze_yaw);
//     float Unit_z1 = Unit_z0;

//     // Multiplying UnitGazeVec_1 by Rotation Y Matrix
//     float UnitGaze_pitch = atan2(Unit_z1, Unit_x1); // rotation about y axis
//     float Unit_x2 = Unit_x1 * cos(UnitGaze_pitch) + Unit_z1 * sin(UnitGaze_pitch);
//     float Unit_y2 = Unit_y1;
//     float Unit_z2 = -Unit_x1 * sin(UnitGaze_pitch) + Unit_z1 * cos(UnitGaze_pitch);

//     // UE_LOG(LogTemp, Log, TEXT("x2 logging %f"), Unit_x2);
//     // UE_LOG(LogTemp, Log, TEXT("y2 logging %f"), Unit_y2);
//     // UE_LOG(LogTemp, Log, TEXT("z2 logging %f"), Unit_z2);

//     // Multiplying RotVec by Rotation Z Matrix
//     float Rot_x1 = Rot_x0 * cos(UnitGaze_yaw) + Rot_y0 * sin(UnitGaze_yaw);
//     float Rot_y1 = -Rot_x0 * sin(UnitGaze_yaw) + Rot_y0 * cos(UnitGaze_yaw);
//     float Rot_z1 = Rot_z0;

//     // Multiplying RotVec_1 by Rotation Y Matrix
//     float Rot_x2 = Rot_x1 * cos(UnitGaze_pitch) + Rot_z1 * sin(UnitGaze_pitch);
//     float Rot_y2 = Rot_y1;
//     float Rot_z2 = -Rot_x1 * sin(UnitGaze_pitch) + Rot_z1 * cos(UnitGaze_pitch);

//     // Get Yaw
//     float yaw = atan2(Rot_y2, Rot_x2);
//     // UE_LOG(LogTemp, Log, TEXT("yaw log: %f"), yaw);
//     // VehicleInputs.yaw = yaw;

//     // Get Pitch
//     float pitch = atan2(Rot_z2, Rot_x2);
//     // UE_LOG(LogTemp, Log, TEXT("pitch log: %f"), pitch);
//     // VehicleInputs.pitch = pitch;

//     return {pitch, yaw};
// }

// FVector AEgoVehicle::GenerateRotVec(FVector UnitGazeVec) const
// {
//     // Normalize input vector
//     UnitGazeVec = UnitGazeVec / UnitGazeVec.Size();
//     float Unit_x0 = UnitGazeVec[0];
//     float Unit_y0 = UnitGazeVec[1];
//     float Unit_z0 = UnitGazeVec[2];

//     float UnitGaze_yaw = atan2(Unit_y0, Unit_x0); // rotation about z axis
//     float Unit_x1 = Unit_x0 * cos(UnitGaze_yaw) + Unit_y0 * sin(UnitGaze_yaw);
//     float Unit_y1 = -Unit_x0 * sin(UnitGaze_yaw) + Unit_y0 * cos(UnitGaze_yaw);
//     float Unit_z1 = Unit_z0;

//     float UnitGaze_pitch = atan2(Unit_z1, Unit_x1); // rotation about y axis
//     float Unit_x2 = Unit_x1 * cos(UnitGaze_pitch) + Unit_z1 * sin(UnitGaze_pitch);
//     float Unit_y2 = Unit_y1;
//     float Unit_z2 = -Unit_x1 * sin(UnitGaze_pitch) + Unit_z1 * cos(UnitGaze_pitch);

//     // (1, 0, 0) Vector
//     // UE_LOG(LogTemp, Log, TEXT("x2 logging %f"), Unit_x2);
//     // UE_LOG(LogTemp, Log, TEXT("y2 logging %f"), Unit_y2);
//     // UE_LOG(LogTemp, Log, TEXT("z2 logging %f"), Unit_z2);

//     float pitch_dist_pos = tan(this->pitchMax + this->vert_offset);
//     float pitch_dist_neg = tan(-this->pitchMax + this->vert_offset);
//     float yaw_dist = tan(this->yawMax);
//     // UE_LOG(LogTemp, Log, TEXT("pitch dist, %f"), pitch_dist);
//     // UE_LOG(LogTemp, Log, TEXT("yaw dist, %f"), yaw_dist);

//     float Rot_x0 = 1.f;
//     float Rot_y0 = FMath::RandRange(-yaw_dist, yaw_dist);
//     float Rot_z0 = FMath::RandRange(pitch_dist_neg, pitch_dist_pos);
//     // UE_LOG(LogTemp, Log, TEXT("Rot_y0, %f"), Rot_y0);
//     // UE_LOG(LogTemp, Log, TEXT("Rot_z0, %f"), Rot_z0);

//     // Inverse rotation matrix to get back to world coordinates
//     float Rot_x1 = Rot_x0 * cos(UnitGaze_pitch) - Rot_z0 * sin(UnitGaze_pitch);
//     float Rot_y1 = Rot_y0;
//     float Rot_z1 = Rot_x0 * sin(UnitGaze_pitch) + Rot_z0 * cos(UnitGaze_pitch);

//     float Rot_x2 = Rot_x1 * cos(UnitGaze_yaw) - Rot_y1 * sin(UnitGaze_yaw);
//     float Rot_y2 = Rot_x1 * sin(UnitGaze_yaw) + Rot_y1 * cos(UnitGaze_yaw);
//     float Rot_z2 = Rot_z1;

//     FVector RotVec = FVector(Rot_x2, Rot_y2, Rot_z2);
//     // normalize
//     RotVec = RotVec / RotVec.Size();

//     return RotVec;
// }

FVector AEgoSensor::GenerateRotVecGivenAngles(const FVector &GazeVec, float yaw, float pitch) const
{
    // Normalize input vector
    const FVector UnitGazeVec = GazeVec / GazeVec.Size();
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
    return RotVec / RotVec.Size(); // normalized
}