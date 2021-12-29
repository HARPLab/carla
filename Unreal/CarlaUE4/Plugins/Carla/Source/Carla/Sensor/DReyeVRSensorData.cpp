#include "DReyeVRSensorData.h"

/// NOTE: provides implementations for both const and non-const return-by-reference getters
// the non-const versions can modify the internal object, while the const versions cannot (for reading only)

namespace DReyeVR
{
const int64_t &SensorData::GetTimestampCarla() const
{
    return TimestampCarla;
}
int64_t &SensorData::GetTimestampCarla()
{
    return TimestampCarla;
}

// from EyeTrackerData
const int64_t &SensorData::GetTimestampDevice() const
{
    return EyeTrackerData.TimestampDevice;
}
int64_t &SensorData::GetTimestampDevice()
{
    return EyeTrackerData.TimestampDevice;
}

const int64_t &SensorData::GetFrameSequence() const
{
    return EyeTrackerData.FrameSequence;
}
int64_t &SensorData::GetFrameSequence()
{
    return EyeTrackerData.FrameSequence;
}

// combined (both) eyes
const float &SensorData::GetEyeVergence() const
{
    return EyeTrackerData.Combined.Vergence;
}
float &SensorData::GetEyeVergence()
{
    return EyeTrackerData.Combined.Vergence;
}

const FVector &SensorData::GetCombinedGazeDir() const
{
    return EyeTrackerData.Combined.GazeDir;
}
FVector &SensorData::GetCombinedGazeDir()
{
    return EyeTrackerData.Combined.GazeDir;
}

const FVector &SensorData::GetCombinedGazeOrigin() const
{
    return EyeTrackerData.Combined.GazeOrigin;
}
FVector &SensorData::GetCombinedGazeOrigin()
{
    return EyeTrackerData.Combined.GazeOrigin;
}

const bool &SensorData::GetCombinedValidity() const
{
    return EyeTrackerData.Combined.GazeValid;
}
bool &SensorData::GetCombinedValidity()
{
    return EyeTrackerData.Combined.GazeValid;
}

// left eye
const FVector &SensorData::GetLeftGazeDir() const
{
    return EyeTrackerData.Left.GazeDir;
}
FVector &SensorData::GetLeftGazeDir()
{
    return EyeTrackerData.Left.GazeDir;
}

const FVector &SensorData::GetLeftGazeOrigin() const
{
    return EyeTrackerData.Left.GazeOrigin;
}
FVector &SensorData::GetLeftGazeOrigin()
{
    return EyeTrackerData.Left.GazeOrigin;
}

const bool &SensorData::GetLeftValidity() const
{
    return EyeTrackerData.Left.GazeValid;
}
bool &SensorData::GetLeftValidity()
{
    return EyeTrackerData.Left.GazeValid;
}

const float &SensorData::GetLeftEyeOpenness() const
{
    return EyeTrackerData.Left.EyeOpenness;
}
float &SensorData::GetLeftEyeOpenness()
{
    return EyeTrackerData.Left.EyeOpenness;
}

const bool &SensorData::GetLeftEyeOpennessValidity() const
{
    return EyeTrackerData.Left.EyeOpennessValid;
}
bool &SensorData::GetLeftEyeOpennessValidity()
{
    return EyeTrackerData.Left.EyeOpennessValid;
}

const float &SensorData::GetLeftPupilDiameter() const
{
    return EyeTrackerData.Left.PupilDiameter;
}
float &SensorData::GetLeftPupilDiameter()
{
    return EyeTrackerData.Left.PupilDiameter;
}

const FVector2D &SensorData::GetLeftPupilPosition() const
{
    return EyeTrackerData.Left.PupilPosition;
}
FVector2D &SensorData::GetLeftPupilPosition()
{
    return EyeTrackerData.Left.PupilPosition;
}

const bool &SensorData::GetLeftPupilPositionValidity() const
{
    return EyeTrackerData.Left.PupilPositionValid;
}
bool &SensorData::GetLeftPupilPositionValidity()
{
    return EyeTrackerData.Left.PupilPositionValid;
}

// right eye
const FVector &SensorData::GetRightGazeDir() const
{
    return EyeTrackerData.Right.GazeDir;
}
FVector &SensorData::GetRightGazeDir()
{
    return EyeTrackerData.Right.GazeDir;
}

const FVector &SensorData::GetRightGazeOrigin() const
{
    return EyeTrackerData.Right.GazeOrigin;
}
FVector &SensorData::GetRightGazeOrigin()
{
    return EyeTrackerData.Right.GazeOrigin;
}

const bool &SensorData::GetRightValidity() const
{
    return EyeTrackerData.Right.GazeValid;
}
bool &SensorData::GetRightValidity()
{
    return EyeTrackerData.Right.GazeValid;
}

const float &SensorData::GetRightEyeOpenness() const
{
    return EyeTrackerData.Right.EyeOpenness;
}
float &SensorData::GetRightEyeOpenness()
{
    return EyeTrackerData.Right.EyeOpenness;
}

const bool &SensorData::GetRightEyeOpennessValidity() const
{
    return EyeTrackerData.Right.EyeOpennessValid;
}
bool &SensorData::GetRightEyeOpennessValidity()
{
    return EyeTrackerData.Right.EyeOpennessValid;
}

const float &SensorData::GetRightPupilDiameter() const
{
    return EyeTrackerData.Right.PupilDiameter;
}
float &SensorData::GetRightPupilDiameter()
{
    return EyeTrackerData.Right.PupilDiameter;
}

const FVector2D &SensorData::GetRightPupilPosition() const
{
    return EyeTrackerData.Right.PupilPosition;
}
FVector2D &SensorData::GetRightPupilPosition()
{
    return EyeTrackerData.Right.PupilPosition;
}

const bool &SensorData::GetRightPupilPositionValidity() const
{
    return EyeTrackerData.Right.PupilPositionValid;
}
bool &SensorData::GetRightPupilPositionValidity()
{
    return EyeTrackerData.Right.PupilPositionValid;
}

// from SensorData
const FVector &SensorData::GetHMDLocation() const
{
    return HMDLocation;
}
FVector &SensorData::GetHMDLocation()
{
    return HMDLocation;
}

const FRotator &SensorData::GetHMDRotation() const
{
    return HMDRotation;
}
FRotator &SensorData::GetHMDRotation()
{
    return HMDRotation;
}

const float &SensorData::GetEgoVelocity() const
{
    return EgoVelocity;
}
float &SensorData::GetEgoVelocity()
{
    return EgoVelocity;
}

const FString &SensorData::GetFocusActorName() const
{
    return FocusActorName;
}
FString &SensorData::GetFocusActorName()
{
    return FocusActorName;
}

const FVector &SensorData::GetFocusActorPoint() const
{
    return FocusActorPoint;
}
FVector &SensorData::GetFocusActorPoint()
{
    return FocusActorPoint;
}

const float &SensorData::GetFocusActorDistance() const
{
    return FocusActorDistance;
}
float &SensorData::GetFocusActorDistance()
{
    return FocusActorDistance;
}

const DReyeVR::UserInputs &SensorData::GetUserInputs() const
{
    return Inputs;
}
DReyeVR::UserInputs &SensorData::GetUserInputs()
{
    return Inputs;
}
}; // namespace DReyeVR