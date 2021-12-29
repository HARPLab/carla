#pragma once
#ifndef DREYEVR_SENSOR_DATA
#define DREYEVR_SENSOR_DATA

#include <chrono>  // timing threads
#include <cstdint> // int64_t
#include <iostream>
#include <string>

/// NOTE: all functions here are inline to avoid nasty linker errors. Though this can
// probably be refactored to have a proper associated .cpp file

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
    float PupilDiameter = 0.f;
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

enum class Gaze
{
    COMBINED, // default for functions
    RIGHT,
    LEFT,
};

enum class Eye
{
    RIGHT, // default for functions
    LEFT,
};

class SensorData // everything being held by this sensor
{
  public:
    SensorData() = default;

    void SetEyeTrackerData(const SRanipalData &NewData)
    {
        EyeTrackerData = NewData;
    }

    /// NOTE: we provide implementations for both const and non-const return-by-reference getters
    // the non-const versions can modify the internal object, while the const versions cannot (for reading only)
    const int64_t &TimestampCarla() const
    {
        return TimestampCarlaUE4;
    }
    int64_t &TimestampCarla()
    {
        return TimestampCarlaUE4;
    }

    // from EyeTrackerData
    const int64_t &TimestampDevice() const
    {
        return EyeTrackerData.TimestampDevice;
    }
    int64_t &TimestampDevice()
    {
        return EyeTrackerData.TimestampDevice;
    }

    const int64_t &FrameSequence() const
    {
        return EyeTrackerData.FrameSequence;
    }
    int64_t &FrameSequence()
    {
        return EyeTrackerData.FrameSequence;
    }

    const float &GazeVergence() const
    {
        return EyeTrackerData.Combined.Vergence;
    }
    float &GazeVergence()
    {
        return EyeTrackerData.Combined.Vergence;
    }

    // All eye data
    const FVector &GazeDir(DReyeVR::Gaze Index = DReyeVR::Gaze::COMBINED) const
    {
        if (Index == DReyeVR::Gaze::LEFT)
            return EyeTrackerData.Left.GazeDir;
        if (Index == DReyeVR::Gaze::RIGHT)
            return EyeTrackerData.Right.GazeDir;
        return EyeTrackerData.Combined.GazeDir;
    }
    FVector &GazeDir(DReyeVR::Gaze Index = DReyeVR::Gaze::COMBINED)
    {
        if (Index == DReyeVR::Gaze::LEFT)
            return EyeTrackerData.Left.GazeDir;
        if (Index == DReyeVR::Gaze::RIGHT)
            return EyeTrackerData.Right.GazeDir;
        return EyeTrackerData.Combined.GazeDir;
    }

    const FVector &GazeOrigin(DReyeVR::Gaze Index = DReyeVR::Gaze::COMBINED) const
    {
        if (Index == DReyeVR::Gaze::LEFT)
            return EyeTrackerData.Left.GazeOrigin;
        if (Index == DReyeVR::Gaze::RIGHT)
            return EyeTrackerData.Right.GazeOrigin;
        return EyeTrackerData.Combined.GazeOrigin;
    }
    FVector &GazeOrigin(DReyeVR::Gaze Index = DReyeVR::Gaze::COMBINED)
    {
        if (Index == DReyeVR::Gaze::LEFT)
            return EyeTrackerData.Left.GazeOrigin;
        if (Index == DReyeVR::Gaze::RIGHT)
            return EyeTrackerData.Right.GazeOrigin;
        return EyeTrackerData.Combined.GazeOrigin;
    }

    const bool &GazeValidity(DReyeVR::Gaze Index = DReyeVR::Gaze::COMBINED) const
    {
        if (Index == DReyeVR::Gaze::LEFT)
            return EyeTrackerData.Left.GazeValid;
        if (Index == DReyeVR::Gaze::RIGHT)
            return EyeTrackerData.Right.GazeValid;
        return EyeTrackerData.Combined.GazeValid;
    }
    bool &GazeValidity(DReyeVR::Gaze Index = DReyeVR::Gaze::COMBINED)
    {
        if (Index == DReyeVR::Gaze::LEFT)
            return EyeTrackerData.Left.GazeValid;
        if (Index == DReyeVR::Gaze::RIGHT)
            return EyeTrackerData.Right.GazeValid;
        return EyeTrackerData.Combined.GazeValid;
    }

    const float &EyeOpenness(DReyeVR::Eye Index = DReyeVR::Eye::RIGHT) const
    {
        if (Index == DReyeVR::Eye::LEFT)
            return EyeTrackerData.Left.EyeOpenness;
        return EyeTrackerData.Right.EyeOpenness;
    }
    float &EyeOpenness(DReyeVR::Eye Index = DReyeVR::Eye::RIGHT)
    {
        if (Index == DReyeVR::Eye::LEFT)
            return EyeTrackerData.Left.EyeOpenness;
        return EyeTrackerData.Right.EyeOpenness;
    }

    const bool &EyeOpennessValidity(DReyeVR::Eye Index = DReyeVR::Eye::RIGHT) const
    {
        if (Index == DReyeVR::Eye::LEFT)
            return EyeTrackerData.Left.EyeOpennessValid;
        return EyeTrackerData.Right.EyeOpennessValid;
    }
    bool &EyeOpennessValidity(DReyeVR::Eye Index = DReyeVR::Eye::RIGHT)
    {
        if (Index == DReyeVR::Eye::LEFT)
            return EyeTrackerData.Left.EyeOpennessValid;
        return EyeTrackerData.Right.EyeOpennessValid;
    }

    const float &PupilDiameter(DReyeVR::Eye Index = DReyeVR::Eye::RIGHT) const
    {
        if (Index == DReyeVR::Eye::LEFT)
            return EyeTrackerData.Left.PupilDiameter;
        return EyeTrackerData.Right.PupilDiameter;
    }
    float &PupilDiameter(DReyeVR::Eye Index = DReyeVR::Eye::RIGHT)
    {
        if (Index == DReyeVR::Eye::LEFT)
            return EyeTrackerData.Left.PupilDiameter;
        return EyeTrackerData.Right.PupilDiameter;
    }

    const FVector2D &PupilPosition(DReyeVR::Eye Index = DReyeVR::Eye::RIGHT) const
    {
        if (Index == DReyeVR::Eye::LEFT)
            return EyeTrackerData.Left.PupilPosition;
        return EyeTrackerData.Right.PupilPosition;
    }
    FVector2D &PupilPosition(DReyeVR::Eye Index = DReyeVR::Eye::RIGHT)
    {
        if (Index == DReyeVR::Eye::LEFT)
            return EyeTrackerData.Left.PupilPosition;
        return EyeTrackerData.Right.PupilPosition;
    }

    const bool &PupilPositionValidity(DReyeVR::Eye Index = DReyeVR::Eye::RIGHT) const
    {
        if (Index == DReyeVR::Eye::LEFT)
            return EyeTrackerData.Left.PupilPositionValid;
        return EyeTrackerData.Right.PupilPositionValid;
    }
    bool &PupilPositionValidity(DReyeVR::Eye Index = DReyeVR::Eye::RIGHT)
    {
        if (Index == DReyeVR::Eye::LEFT)
            return EyeTrackerData.Left.PupilPositionValid;
        return EyeTrackerData.Right.PupilPositionValid;
    }

    // from SensorData
    const FVector &HMDLocation() const
    {
        return CameraLocation;
    }
    FVector &HMDLocation()
    {
        return CameraLocation;
    }

    const FRotator &HMDRotation() const
    {
        return CameraRotation;
    }
    FRotator &HMDRotation()
    {
        return CameraRotation;
    }

    const float &EgoVelocity() const
    {
        return EgoVehicleVelocity;
    }
    float &EgoVelocity()
    {
        return EgoVehicleVelocity;
    }

    const FString &FocusActorName() const
    {
        return FocusActorNameTag;
    }
    FString &FocusActorName()
    {
        return FocusActorNameTag;
    }

    const FVector &FocusActorPoint() const
    {
        return FocusActorHitPoint;
    }
    FVector &FocusActorPoint()
    {
        return FocusActorHitPoint;
    }

    const float &FocusActorDistance() const
    {
        return FocusActorHitDistance;
    }
    float &FocusActorDistance()
    {
        return FocusActorHitDistance;
    }

    const DReyeVR::UserInputs &UserInputs() const
    {
        return Inputs;
    }
    DReyeVR::UserInputs &UserInputs()
    {
        return Inputs;
    }

  private:
    int64_t TimestampCarlaUE4 = 0; // Carla Timestamp (EgoSensor Tick() event) in milliseconds

    SRanipalData EyeTrackerData;
    // HMD position and orientation
    FVector CameraLocation = FVector::ZeroVector;    // initialized as {0,0,0}
    FRotator CameraRotation = FRotator::ZeroRotator; // initialized to {0,0,0}
    // Ego variables
    float EgoVehicleVelocity;
    // FFocusInfo data
    FString FocusActorNameTag = "None";               // Tag of the actor being focused on
    FVector FocusActorHitPoint = FVector::ZeroVector; // Hit point of the Focus Actor
    float FocusActorHitDistance = 0.f;                // Distance to the Focus Actor
    // User inputs
    struct UserInputs Inputs;
};
}; // namespace DReyeVR

#endif