#pragma once
#ifndef DREYEVR_SENSOR_DATA
#define DREYEVR_SENSOR_DATA

#include "Carla/Recorder/CarlaRecorderHelpers.h" // WriteValue, WriteFVector, WriteFString, ...
#include <chrono>                                // timing threads
#include <cstdint>                               // int64_t
#include <fstream>
#include <iostream>
#include <sstream>
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
    void Read(std::ifstream &InFile)
    {
        ReadFVector(InFile, GazeDir);
        ReadFVector(InFile, GazeOrigin);
        ReadValue<bool>(InFile, GazeValid);
    }
    void Write(std::ofstream &OutFile) const
    {
        WriteFVector(OutFile, GazeDir);
        WriteFVector(OutFile, GazeOrigin);
        WriteValue<bool>(OutFile, GazeValid);
    }
};

struct CombinedEyeData : EyeData
{
    float Vergence = 0.f; // in cm (default UE4 units)
    void Read(std::ifstream &InFile)
    {
        EyeData::Read(InFile);
        ReadValue<float>(InFile, Vergence);
    }
    void Write(std::ofstream &OutFile) const
    {
        EyeData::Write(OutFile);
        WriteValue<float>(OutFile, Vergence);
    }
};

struct SingleEyeData : EyeData
{
    float EyeOpenness = 0.f;
    bool EyeOpennessValid = false;
    float PupilDiameter = 0.f;
    FVector2D PupilPosition = FVector2D::ZeroVector;
    bool PupilPositionValid = false;

    void Read(std::ifstream &InFile)
    {
        EyeData::Read(InFile);
        ReadValue<float>(InFile, EyeOpenness);
        ReadValue<bool>(InFile, EyeOpennessValid);
        ReadValue<float>(InFile, PupilDiameter);
        ReadFVector2D(InFile, PupilPosition);
        ReadValue<bool>(InFile, PupilPositionValid);
    }
    void Write(std::ofstream &OutFile) const
    {
        EyeData::Write(OutFile);
        WriteValue<float>(OutFile, EyeOpenness);
        WriteValue<bool>(OutFile, EyeOpennessValid);
        WriteValue<float>(OutFile, PupilDiameter);
        WriteFVector2D(OutFile, PupilPosition);
        WriteValue<bool>(OutFile, PupilPositionValid);
    }
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

    void Read(std::ifstream &InFile)
    {
        ReadValue<float>(InFile, Throttle);
        ReadValue<float>(InFile, Steering);
        ReadValue<float>(InFile, Brake);
        ReadValue<bool>(InFile, ToggledReverse);
        ReadValue<bool>(InFile, TurnSignalLeft);
        ReadValue<bool>(InFile, TurnSignalRight);
        ReadValue<bool>(InFile, HoldHandbrake);
    }
    void Write(std::ofstream &OutFile) const
    {
        WriteValue<float>(OutFile, Throttle);
        WriteValue<float>(OutFile, Steering);
        WriteValue<float>(OutFile, Brake);
        WriteValue<bool>(OutFile, ToggledReverse);
        WriteValue<bool>(OutFile, TurnSignalLeft);
        WriteValue<bool>(OutFile, TurnSignalRight);
        WriteValue<bool>(OutFile, HoldHandbrake);
    }
};

struct FocusInfo
{
    // substitute for SRanipal FFocusInfo in SRanipal_Eyes_Enums.h
    TWeakObjectPtr<class AActor> Actor;
    bool bDidHit;
    float Distance;
    FVector HitPointRelative;
    FVector HitPointAbsolute;
    FVector Normal;
    FString ActorNameTag = "None"; // Tag of the actor being focused on

    void Read(std::ifstream &InFile)
    {
        ReadFString(InFile, ActorNameTag);
        ReadValue<bool>(InFile, bDidHit);
        ReadFVector(InFile, HitPointRelative);
        ReadFVector(InFile, HitPointAbsolute);
        ReadFVector(InFile, Normal);
        ReadValue<float>(InFile, Distance);
    }
    void Write(std::ofstream &OutFile) const
    {
        WriteFString(OutFile, ActorNameTag);
        WriteValue<bool>(OutFile, bDidHit);
        WriteFVector(OutFile, HitPointRelative);
        WriteFVector(OutFile, HitPointAbsolute);
        WriteFVector(OutFile, Normal);
        WriteValue<float>(OutFile, Distance);
    }
};

struct SRanipalData
{
    int64_t TimestampDevice = 0; // timestamp of when the frame was captured by SRanipal in milliseconds
    int64_t FrameSequence = 0;   // Frame sequence of SRanipal
    CombinedEyeData Combined;
    SingleEyeData Left;
    SingleEyeData Right;
    void Read(std::ifstream &InFile)
    {
        ReadValue<int64_t>(InFile, TimestampDevice);
        ReadValue<int64_t>(InFile, FrameSequence);
        Combined.Read(InFile);
        Left.Read(InFile);
        Right.Read(InFile);
    }
    void Write(std::ofstream &OutFile) const
    {
        WriteValue<int64_t>(OutFile, TimestampDevice);
        WriteValue<int64_t>(OutFile, FrameSequence);
        Combined.Write(OutFile);
        Left.Write(OutFile);
        Right.Write(OutFile);
    }
};

enum class Gaze
{
    COMBINED, // default for functions
    RIGHT,
    LEFT,
};

enum class Eye
{
    RIGHT,
    LEFT,
};

class SensorData // everything being held by this sensor
{
  public:
    SensorData() = default;
    /////////////////////////:GETTERS://////////////////////////////

    int64_t GetTimestampCarla() const
    {
        return TimestampCarlaUE4;
    }
    int64_t GetTimestampDevice() const
    {
        return EyeTrackerData.TimestampDevice;
    }
    int64_t GetFrameSequence() const
    {
        return EyeTrackerData.FrameSequence;
    }
    float GetGazeVergence() const
    {
        return EyeTrackerData.Combined.Vergence; // in cm (default UE4 units)
    }
    const FVector &GetGazeDir(DReyeVR::Gaze Index = DReyeVR::Gaze::COMBINED) const
    {
        switch (Index)
        {
        case DReyeVR::Gaze::LEFT:
            return EyeTrackerData.Left.GazeDir;
        case DReyeVR::Gaze::RIGHT:
            return EyeTrackerData.Right.GazeDir;
        case DReyeVR::Gaze::COMBINED:
            return EyeTrackerData.Combined.GazeDir;
        }
    }
    const FVector &GetGazeOrigin(DReyeVR::Gaze Index = DReyeVR::Gaze::COMBINED) const
    {
        switch (Index)
        {
        case DReyeVR::Gaze::LEFT:
            return EyeTrackerData.Left.GazeOrigin;
        case DReyeVR::Gaze::RIGHT:
            return EyeTrackerData.Right.GazeOrigin;
        case DReyeVR::Gaze::COMBINED:
            return EyeTrackerData.Combined.GazeOrigin;
        }
    }
    bool GetGazeValidity(DReyeVR::Gaze Index = DReyeVR::Gaze::COMBINED) const
    {
        switch (Index)
        {
        case DReyeVR::Gaze::LEFT:
            return EyeTrackerData.Left.GazeValid;
        case DReyeVR::Gaze::RIGHT:
            return EyeTrackerData.Right.GazeValid;
        case DReyeVR::Gaze::COMBINED:
            return EyeTrackerData.Combined.GazeValid;
        }
    }
    float GetEyeOpenness(DReyeVR::Eye Index) const // returns eye openness as a percentage [0,1]
    {
        switch (Index)
        {
        case DReyeVR::Eye::LEFT:
            return EyeTrackerData.Left.EyeOpenness;
        case DReyeVR::Eye::RIGHT:
            return EyeTrackerData.Right.EyeOpenness;
        }
    }
    bool GetEyeOpennessValidity(DReyeVR::Eye Index) const
    {
        switch (Index)
        {
        case DReyeVR::Eye::LEFT:
            return EyeTrackerData.Left.EyeOpennessValid;
        case DReyeVR::Eye::RIGHT:
            return EyeTrackerData.Right.EyeOpennessValid;
        }
    }
    float GetPupilDiameter(DReyeVR::Eye Index) const // returns diameter in mm
    {
        switch (Index)
        {
        case DReyeVR::Eye::LEFT:
            return EyeTrackerData.Left.PupilDiameter;
        case DReyeVR::Eye::RIGHT:
            return EyeTrackerData.Right.PupilDiameter;
        }
    }
    const FVector2D &GetPupilPosition(DReyeVR::Eye Index) const
    {
        switch (Index)
        {
        case DReyeVR::Eye::LEFT:
            return EyeTrackerData.Left.PupilPosition;
        case DReyeVR::Eye::RIGHT:
            return EyeTrackerData.Right.PupilPosition;
        }
    }
    bool GetPupilPositionValidity(DReyeVR::Eye Index) const
    {
        switch (Index)
        {
        case DReyeVR::Eye::LEFT:
            return EyeTrackerData.Left.PupilPositionValid;
        case DReyeVR::Eye::RIGHT:
            return EyeTrackerData.Right.PupilPositionValid;
        }
    }

    // from SensorData
    const FVector &GetHMDLocation() const
    {
        return CameraLocation;
    }
    const FRotator &GetHMDRotation() const
    {
        return CameraRotation;
    }
    float GetEgoVelocity() const
    {
        return EgoVehicleVelocity; // returns ego velocity in cm/s
    }
    const FString &GetFocusActorName() const
    {
        return FocusData.ActorNameTag;
    }
    const FVector &GetFocusActorPoint() const
    {
        return FocusData.HitPointAbsolute;
    }
    float GetFocusActorDistance() const
    {
        return FocusData.Distance;
    }
    const DReyeVR::UserInputs &GetUserInputs() const
    {
        return Inputs;
    }
    ////////////////////:SETTERS://////////////////////

    void UpdateCamera(const FVector &NewCameraPose, const FRotator &NewCameraRot)
    {
        CameraLocation = NewCameraPose;
        CameraRotation = NewCameraRot;
    }

    void Update(int64_t NewTimestamp, struct SRanipalData &NewEyeData, const FVector &NewCameraLoc,
                const FRotator &NewCameraRot, float NewVelocity, struct FocusInfo &NewFocus,
                struct UserInputs &NewInputs)
    {
        TimestampCarlaUE4 = std::max(NewTimestamp, 0l);
        EyeTrackerData = NewEyeData;
        CameraLocation = NewCameraLoc;
        CameraRotation = NewCameraRot;
        EgoVehicleVelocity = NewVelocity;
        FocusData = NewFocus;
        Inputs = NewInputs;
    }

    ////////////////////:SERIALIZATION://////////////////////
    void Read(std::ifstream &InFile)
    {
        /// CAUTION: make sure the order of writes/reads is the same
        ReadValue<int64_t>(InFile, TimestampCarlaUE4);
        ReadFVector(InFile, CameraLocation);
        ReadFRotator(InFile, CameraRotation);
        ReadValue<float>(InFile, EgoVehicleVelocity);
        EyeTrackerData.Read(InFile);
        FocusData.Read(InFile);
        Inputs.Read(InFile);
    }

    void Write(std::ofstream &OutFile) const
    {
        /// CAUTION: make sure the order of writes/reads is the same
        WriteValue<int64_t>(OutFile, GetTimestampCarla());
        WriteFVector(OutFile, GetHMDLocation());
        WriteFRotator(OutFile, GetHMDRotation());
        WriteValue<float>(OutFile, GetEgoVelocity());
        EyeTrackerData.Write(OutFile);
        FocusData.Write(OutFile);
        Inputs.Write(OutFile);
    }

    std::string ToString() const
    {
        struct // overloaded lambdas to print UE4 types
        {
            std::string operator()(const FVector &F)
            {
                return (*this)(F.ToString());
            }
            std::string operator()(const FRotator &F)
            {
                return (*this)(F.ToString());
            }
            std::string operator()(const FVector2D &F)
            {
                return (*this)(F.ToString());
            }
            std::string operator()(const FString &F)
            {
                return TCHAR_TO_UTF8(*F);
            }
        } ToStdStr;

        const std::string sep = ","; // comma separated
        std::stringstream out;
        out << "T_Device:" << GetTimestampDevice() << sep                             // Sranipal time
            << "T_Carla:" << GetTimestampCarla() << sep                               // Carla time
            << "FrameSeq:" << GetFrameSequence() << sep                               // SRanipal Framesequence
            << "|EGOVEHICLE|"                                                         // Ego Vehicle fields
            << "HMDLoc:" << ToStdStr(GetHMDLocation()) << sep                         // HMD location
            << "HMDRot:" << ToStdStr(GetHMDRotation()) << sep                         // HMD rotation
            << "EgoVel:" << GetEgoVelocity() << sep                                   // Ego Velocity
            << "|COMBINED|"                                                           // Combined Gaze fields
            << "GazeDir:" << ToStdStr(GetGazeDir()) << sep                            // Gaze dir vector
            << "GazeOrigin:" << ToStdStr(GetGazeOrigin()) << sep                      // Combined Eye origin vector
            << "Vergence:" << GetGazeVergence() << sep                                // Calculated vergence
            << "|LEFT|"                                                               // LEFT gaze/eye fields
            << "LGazeDir:" << ToStdStr(GetGazeDir(DReyeVR::Gaze::LEFT)) << sep        // LEFT gaze dir
            << "LEyeOrigin:" << ToStdStr(GetGazeOrigin(DReyeVR::Gaze::LEFT)) << sep   // LEFT gaze dir
            << "LOpenness:" << GetEyeOpenness(DReyeVR::Eye::LEFT) << sep              // LEFT eye openness
            << "LPupilPos:" << ToStdStr(GetPupilPosition(DReyeVR::Eye::LEFT)) << sep  // LEFT pupil position
            << "LPupilDiam:" << GetPupilDiameter(DReyeVR::Eye::LEFT) << sep           // LEFT pupil diameter
            << "|RIGHT|"                                                              // RIGHT gaze/eye fields
            << "RGazeDir:" << ToStdStr(GetGazeDir(DReyeVR::Gaze::RIGHT)) << sep       // RIGHT gaze dir
            << "REyeOrigin:" << ToStdStr(GetGazeOrigin(DReyeVR::Gaze::RIGHT)) << sep  // RIGHT gaze dir
            << "ROpenness:" << GetEyeOpenness(DReyeVR::Eye::RIGHT) << sep             // RIGHT eye openness
            << "RPupilPos:" << ToStdStr(GetPupilPosition(DReyeVR::Eye::RIGHT)) << sep // RIGHT pupil position
            << "RPupilDiam:" << GetPupilDiameter(DReyeVR::Eye::RIGHT) << sep          // LEFT pupil diameter
            << "|FOCUSINFO|"                                                          // Focus info
            << "FActorName:" << ToStdStr(GetFocusActorName()) << sep                  // Name (tag) of focus actor
            << "FActorPoint:" << ToStdStr(GetFocusActorPoint()) << sep                // Location of focused actor
            << "FActorDist:" << GetFocusActorDistance() << sep                        // Distance to focused actor
            << "|VALIDITY|"                                                           // validity booleans
            << "GazeV:" << GetGazeValidity() << sep                                   // validity for COMBINE gaze
            << "RGazeV:" << GetGazeValidity(DReyeVR::Gaze::RIGHT) << sep              // validity for RIGHT gaze
            << "REyeOpenV:" << GetEyeOpennessValidity(DReyeVR::Eye::RIGHT) << sep     // validity for RIGHT openness
            << "RPupilPosV:" << GetPupilPositionValidity(DReyeVR::Eye::RIGHT) << sep  // validity for RIGHT pupil pos
            << "LGazeV:" << GetGazeValidity(DReyeVR::Gaze::LEFT) << sep               // validity for LEFT gaze
            << "LEyeOpenV:" << GetEyeOpennessValidity(DReyeVR::Eye::LEFT) << sep      // validity for LEFT openness
            << "LPupilPosV:" << GetPupilPositionValidity(DReyeVR::Eye::LEFT) << sep   // validity for LEFT pupil pos
            << "|INPUTS|"                                                             // User inputs
            << "Throttle:" << GetUserInputs().Throttle << sep                         // Value of Throttle
            << "Steering:" << GetUserInputs().Steering << sep                         // Value of Steering
            << "Brake:" << GetUserInputs().Brake << sep                               // Value of brake
            << "ToggleRev:" << GetUserInputs().ToggledReverse << sep                  // toggled reverse
            << "TurnSignalLeft:" << GetUserInputs().TurnSignalLeft << sep             // enabled left turn signal
            << "TurnSignalRight:" << GetUserInputs().TurnSignalRight << sep           // enabled right turn signal
            << "Handbrake:" << GetUserInputs().HoldHandbrake << sep                   // the handbrake is held
            ;
        return out.str();
    }

  private:
    int64_t TimestampCarlaUE4; // Carla Timestamp (EgoSensor Tick() event) in milliseconds

    struct SRanipalData EyeTrackerData;
    // Relative HMD position and orientation
    FVector CameraLocation = FVector::ZeroVector;
    FRotator CameraRotation = FRotator::ZeroRotator;
    // Ego variables
    float EgoVehicleVelocity; // note this is in cm/s (default UE4 units)
    // Focus data
    struct FocusInfo FocusData;
    // User inputs
    struct UserInputs Inputs;
};
}; // namespace DReyeVR

#endif