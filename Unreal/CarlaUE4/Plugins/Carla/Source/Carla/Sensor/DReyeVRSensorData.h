#pragma once

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
        Combined.Origin = FVector(-5.60, 0.14, 1.12);
        Left.Origin = FVector(-6.308, 3.247, 1.264);
        Right.Origin = FVector(-5.284, -3.269, 1.014);
    }
    int64_t TimestampDevice = 0; // timestamp of when the frame was captured by SRanipal in milliseconds
    int64_t FrameSequence = 0;   // Frame sequence of SRanipal
    CombinedEyeData Combined;
    SingleEyeData Left, Right;
};

class DReyeVRSensorData // everything being held by this sensor
{
  public:
    DReyeVRSensorData()
    {
    }

    void UpdateEyeTrackerData(const SRanipalData &NewData)
    {
        EyeTrackerData = NewData;
    }

    int64_t GetTimestampCarla() const;
    // from EyeTrackerData
    int64_t GetTimestampDevice() const;
    int64_t GetFrameSequence() const;
    // combined (both) eyes
    float GetEyeVergence() const;
    FVector GetCombinedGazeDir() const;
    FVector GetCombinedGazeOrigin() const;
    bool GetCombinedValidity() const;
    // left eye
    FVector GetLeftGazeDir() const;
    FVector GetLeftGazeOrigin() const;
    bool GetLeftValidity() const;
    float GetLeftEyeOpenness() const;
    bool GetLeftEyeOpennessValidity() const;
    float GetLeftPupilDiameter() const;
    FVector2D GetLeftPupilPosition() const;
    bool GetLeftPupilPositionValidity() const;
    // right eye
    FVector GetRightGazeDir() const;
    FVector GetRightGazeOrigin() const;
    bool GetRightValidity() const;
    float GetRightEyeOpenness() const;
    bool GetRightEyeOpennessValidity() const;
    float GetRightPupilDiameter() const;
    FVector2D GetRightPupilPosition() const;
    bool GetRightPupilPositionValidity() const;
    // from DReyeVRSensorData
    FVector GetHMDLocation() const;
    FRotator GetHMDRotation() const;
    float GetEgoVelocity() const;
    FString GetFocusActorName() const;
    FVector GetFocusActorPoint() const;
    float GetFocusActorDistance() const;
    DReyeVR::UserInputs GetUserInputs() const;

  private:
    int64_t TimestampCarla = 0; // Carla Timestamp (EgoSensor Tick() event) in milliseconds

    SRanipalData EyeTrackerData;
    // HMD position and orientation
    FVector HMDLocation = FVector::ZeroVector;    // initialized as {0,0,0}
    FRotator HMDRotation = FRotator::ZeroRotator; // initialized to {0,0,0}
    // Ego variables
    float Velocity;
    // FFocusInfo data
    FString FocusActorName = "None";               // Tag of the actor being focused on
    FVector FocusActorPoint = FVector::ZeroVector; // Hit point of the Focus Actor
    float FocusActorDistance = 0.f;                // Distance to the Focus Actor
    // User inputs
    struct UserInputs Inputs;
};
}; // namespace DReyeVR