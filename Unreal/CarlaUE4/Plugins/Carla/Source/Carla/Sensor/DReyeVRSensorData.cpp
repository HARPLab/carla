#include "DReyeVRSensorData.h"

namespace DReyeVR
{
int64_t DReyeVRSensorData::GetTimestampCarla() const
{
    return TimestampCarla;
}

// from EyeTrackerData
int64_t DReyeVRSensorData::GetTimestampDevice() const
{
    return EyeTrackerData.TimestampDevice;
}

int64_t DReyeVRSensorData::GetFrameSequence() const
{
    return EyeTrackerData.FrameSequence;
}

// combined (both) eyes
float DReyeVRSensorData::GetEyeVergence() const
{
    return EyeTrackerData.Combined.Vergence;
}

FVector DReyeVRSensorData::GetCombinedGazeDir() const
{
    return EyeTrackerData.Combined.GazeDir;
}

FVector DReyeVRSensorData::GetCombinedGazeOrigin() const
{
    return EyeTracker.Combined.GazeOrigin;
}

bool DReyeVRSensorData::GetCombinedValidity() const
{
    return EyeTracker.Combined.GazeValid;
}

// left eye
FVector DReyeVRSensorData::GetLeftGazeDir() const
{
    return EyeTracker.Left.GazeDir;
}

FVector DReyeVRSensorData::GetLeftGazeOrigin() const
{
    return EyeTracker.Left.GazeOrigin;
}

bool DReyeVRSensorData::GetLeftValidity() const
{
    return EyeTracker.Left.GazeValid;
}

float DReyeVRSensorData::GetLeftEyeOpenness() const
{
    return EyeTracker.Left.EyeOpenness;
}

bool DReyeVRSensorData::GetLeftEyeOpennessValidity() const
{
    return EyeTracker.Left.EyeOpennesValid;
}

float DReyeVRSensorData::GetLeftPupilDiameter() const
{
    return EyeTracker.Left.PupilDiameter;
}

FVector2D DReyeVRSensorData::GetLeftPupilPosition() const
{
    return EyeTracker.Left.PupilPosition;
}

bool DReyeVRSensorData::GetLeftPupilPositionValidity() const
{
    return EyeTracker.Left.PupilPositionValid;
}

// right eye
FVector DReyeVRSensorData::GetRightGazeDir() const
{
    return EyeTracker.Right.GazeDir;
}

FVector DReyeVRSensorData::GetRightGazeOrigin() const
{
    return EyeTracker.Right.GazeOrigin;
}

bool DReyeVRSensorData::GetRightValidity() const
{
    return EyeTracker.Right.GazeValid;
}

float DReyeVRSensorData::GetRightEyeOpenness() const
{
    return EyeTracker.Right.EyeOpenness;
}

bool DReyeVRSensorData::GetRightEyeOpennessValidity() const
{
    return EyeTracker.Right.EyeOpennesValid;
}

float DReyeVRSensorData::GetRightPupilDiameter() const
{
    return EyeTracker.Right.PupilDiameter;
}

FVector2D DReyeVRSensorData::GetRightPupilPosition() const
{
    return EyeTracker.Right.PupilPosition;
}

bool DReyeVRSensorData::GetRightPupilPositionValidity() const
{
    return EyeTracker.Right.PupilPositionValid;
}

// from DReyeVRSensorData
FVector DReyeVRSensorData::GetHMDLocation() const
{
    return HMDLocation;
}

FRotator DReyeVRSensorData::GetHMDRotation() const
{
    return HMDRotation;
}

float DReyeVRSensorData::GetEgoVelocity() const
{
    return EgoVelocity;
}

FString DReyeVRSensorData::GetFocusActorName() const
{
    return FocusActorName;
}

FVector DReyeVRSensorData::GetFocusActorPoint() const
{
    return FocusActorPoint;
}

float DReyeVRSensorData::GetFocusActorDist() const
{
    return FocusActorDistance;
}

DReyeVR::UserInputs DReyeVRSensorData::GetUserInputs() const
{
    return Inputs;
}
}; // namespace DReyeVR