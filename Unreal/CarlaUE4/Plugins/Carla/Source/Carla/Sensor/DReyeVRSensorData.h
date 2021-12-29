#pragma once
#ifndef DREYEVR_SENSOR_DATA
#define DREYEVR_SENSOR_DATA

#include <chrono>  // timing threads
#include <cstdint> // int64_t
#include <iostream>
#include <string>

namespace DReyeVR
{
struct EyeData
{
    FVector GazeDir = FVector::ZeroVector;
    FVector GazeOrigin = FVector::ZeroVector;
    bool GazeValid = false;
};

struct CombinedEyeData : EyeData
{
    float Vergence = 0.f;
};

struct SingleEyeData : EyeData
{
    float EyeOpenness = 0.f;
    bool EyeOpennessValid = false;
    float PupilDiameter = false;
    FVector2D PupilPosition = FVector2D::ZeroVector;
    bool PupilPositionValid = false;
};

struct UserInputs
{
    // User inputs
    float Throttle = 0.f;
    float Steering = 0.f;
    float Brake = 0.f;
    bool ToggledReverse = false;
    bool TurnSignalLeft = false;
    bool TurnSignalRight = false;
    bool HoldHandbrake = false;
    // Add more inputs here!

    // helper functions
    void Clear()
    {
        // clear all inputs to their original values
        Throttle = 0.f;
        Steering = 0.f;
        Brake = 0.f;
        ToggledReverse = false;
        TurnSignalLeft = false;
        TurnSignalRight = false;
        HoldHandbrake = false;
    }
};

struct FocusInfo
{
    // substitute for SRanipal FFocusInfo in SRanipal_Eyes_Enums.h
    TWeakObjectPtr<class AActor> Actor;
    float Distance;
    FVector Point;
    FVector Normal;
};

struct SRanipalData
{
    SRanipalData()
    {
        // Assign default values to SensorData
        Combined.GazeOrigin = FVector(-5.60, 0.14, 1.12);
        Left.GazeOrigin = FVector(-6.308, 3.247, 1.264);
        Right.GazeOrigin = FVector(-5.284, -3.269, 1.014);
    }
    int64_t TimestampDevice = 0; // timestamp of when the frame was captured by SRanipal in milliseconds
    int64_t FrameSequence = 0;   // Frame sequence of SRanipal
    CombinedEyeData Combined;
    SingleEyeData Left, Right;
};

class SensorData // everything being held by this sensor
{
  public:
    SensorData()
    {
    }

    void UpdateEyeTrackerData(const SRanipalData &NewData)
    {
        EyeTrackerData = NewData;
    }

    /// NOTE: we provide implementations for both const and non-const return-by-reference getters
    // the non-const versions can modify the internal object, while the const versions cannot (for reading only)
    const int64_t &GetTimestampCarla() const
    {
        return TimestampCarla;
    }
    int64_t &GetTimestampCarla()
    {
        return TimestampCarla;
    }

    // from EyeTrackerData
    const int64_t &GetTimestampDevice() const
    {
        return EyeTrackerData.TimestampDevice;
    }
    int64_t &GetTimestampDevice()
    {
        return EyeTrackerData.TimestampDevice;
    }

    const int64_t &GetFrameSequence() const
    {
        return EyeTrackerData.FrameSequence;
    }
    int64_t &GetFrameSequence()
    {
        return EyeTrackerData.FrameSequence;
    }

    // combined (both) eyes
    const float &GetEyeVergence() const
    {
        return EyeTrackerData.Combined.Vergence;
    }
    float &GetEyeVergence()
    {
        return EyeTrackerData.Combined.Vergence;
    }

    const FVector &GetCombinedGazeDir() const
    {
        return EyeTrackerData.Combined.GazeDir;
    }
    FVector &GetCombinedGazeDir()
    {
        return EyeTrackerData.Combined.GazeDir;
    }

    const FVector &GetCombinedGazeOrigin() const
    {
        return EyeTrackerData.Combined.GazeOrigin;
    }
    FVector &GetCombinedGazeOrigin()
    {
        return EyeTrackerData.Combined.GazeOrigin;
    }

    const bool &GetCombinedValidity() const
    {
        return EyeTrackerData.Combined.GazeValid;
    }
    bool &GetCombinedValidity()
    {
        return EyeTrackerData.Combined.GazeValid;
    }

    // left eye
    const FVector &GetLeftGazeDir() const
    {
        return EyeTrackerData.Left.GazeDir;
    }
    FVector &GetLeftGazeDir()
    {
        return EyeTrackerData.Left.GazeDir;
    }

    const FVector &GetLeftGazeOrigin() const
    {
        return EyeTrackerData.Left.GazeOrigin;
    }
    FVector &GetLeftGazeOrigin()
    {
        return EyeTrackerData.Left.GazeOrigin;
    }

    const bool &GetLeftValidity() const
    {
        return EyeTrackerData.Left.GazeValid;
    }
    bool &GetLeftValidity()
    {
        return EyeTrackerData.Left.GazeValid;
    }

    const float &GetLeftEyeOpenness() const
    {
        return EyeTrackerData.Left.EyeOpenness;
    }
    float &GetLeftEyeOpenness()
    {
        return EyeTrackerData.Left.EyeOpenness;
    }

    const bool &GetLeftEyeOpennessValidity() const
    {
        return EyeTrackerData.Left.EyeOpennessValid;
    }
    bool &GetLeftEyeOpennessValidity()
    {
        return EyeTrackerData.Left.EyeOpennessValid;
    }

    const float &GetLeftPupilDiameter() const
    {
        return EyeTrackerData.Left.PupilDiameter;
    }
    float &GetLeftPupilDiameter()
    {
        return EyeTrackerData.Left.PupilDiameter;
    }

    const FVector2D &GetLeftPupilPosition() const
    {
        return EyeTrackerData.Left.PupilPosition;
    }
    FVector2D &GetLeftPupilPosition()
    {
        return EyeTrackerData.Left.PupilPosition;
    }

    const bool &GetLeftPupilPositionValidity() const
    {
        return EyeTrackerData.Left.PupilPositionValid;
    }
    bool &GetLeftPupilPositionValidity()
    {
        return EyeTrackerData.Left.PupilPositionValid;
    }

    // right eye
    const FVector &GetRightGazeDir() const
    {
        return EyeTrackerData.Right.GazeDir;
    }
    FVector &GetRightGazeDir()
    {
        return EyeTrackerData.Right.GazeDir;
    }

    const FVector &GetRightGazeOrigin() const
    {
        return EyeTrackerData.Right.GazeOrigin;
    }
    FVector &GetRightGazeOrigin()
    {
        return EyeTrackerData.Right.GazeOrigin;
    }

    const bool &GetRightValidity() const
    {
        return EyeTrackerData.Right.GazeValid;
    }
    bool &GetRightValidity()
    {
        return EyeTrackerData.Right.GazeValid;
    }

    const float &GetRightEyeOpenness() const
    {
        return EyeTrackerData.Right.EyeOpenness;
    }
    float &GetRightEyeOpenness()
    {
        return EyeTrackerData.Right.EyeOpenness;
    }

    const bool &GetRightEyeOpennessValidity() const
    {
        return EyeTrackerData.Right.EyeOpennessValid;
    }
    bool &GetRightEyeOpennessValidity()
    {
        return EyeTrackerData.Right.EyeOpennessValid;
    }

    const float &GetRightPupilDiameter() const
    {
        return EyeTrackerData.Right.PupilDiameter;
    }
    float &GetRightPupilDiameter()
    {
        return EyeTrackerData.Right.PupilDiameter;
    }

    const FVector2D &GetRightPupilPosition() const
    {
        return EyeTrackerData.Right.PupilPosition;
    }
    FVector2D &GetRightPupilPosition()
    {
        return EyeTrackerData.Right.PupilPosition;
    }

    const bool &GetRightPupilPositionValidity() const
    {
        return EyeTrackerData.Right.PupilPositionValid;
    }
    bool &GetRightPupilPositionValidity()
    {
        return EyeTrackerData.Right.PupilPositionValid;
    }

    // from SensorData
    const FVector &GetHMDLocation() const
    {
        return HMDLocation;
    }
    FVector &GetHMDLocation()
    {
        return HMDLocation;
    }

    const FRotator &GetHMDRotation() const
    {
        return HMDRotation;
    }
    FRotator &GetHMDRotation()
    {
        return HMDRotation;
    }

    const float &GetEgoVelocity() const
    {
        return EgoVelocity;
    }
    float &GetEgoVelocity()
    {
        return EgoVelocity;
    }

    const FString &GetFocusActorName() const
    {
        return FocusActorName;
    }
    FString &GetFocusActorName()
    {
        return FocusActorName;
    }

    const FVector &GetFocusActorPoint() const
    {
        return FocusActorPoint;
    }
    FVector &GetFocusActorPoint()
    {
        return FocusActorPoint;
    }

    const float &GetFocusActorDistance() const
    {
        return FocusActorDistance;
    }
    float &GetFocusActorDistance()
    {
        return FocusActorDistance;
    }

    const DReyeVR::UserInputs &GetUserInputs() const
    {
        return Inputs;
    }
    DReyeVR::UserInputs &GetUserInputs()
    {
        return Inputs;
    }

  private:
    int64_t TimestampCarla = 0; // Carla Timestamp (EgoSensor Tick() event) in milliseconds

    SRanipalData EyeTrackerData;
    // HMD position and orientation
    FVector HMDLocation = FVector::ZeroVector;    // initialized as {0,0,0}
    FRotator HMDRotation = FRotator::ZeroRotator; // initialized to {0,0,0}
    // Ego variables
    float EgoVelocity;
    // FFocusInfo data
    FString FocusActorName = "None";               // Tag of the actor being focused on
    FVector FocusActorPoint = FVector::ZeroVector; // Hit point of the Focus Actor
    float FocusActorDistance = 0.f;                // Distance to the Focus Actor
    // User inputs
    struct UserInputs Inputs;
};
}; // namespace DReyeVR

#endif