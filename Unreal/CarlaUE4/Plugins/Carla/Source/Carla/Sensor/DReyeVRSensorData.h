#pragma once

#include <cstdint> // int64_t
#include <iostream>
#include <string>

namespace DReyeVR
{
struct EyeData
{
    FVector GazeRay = FVector::ZeroVector;
    FVector Origin = FVector::ZeroVector;
    bool GazeValid = false;
};

struct ComboEyeData : EyeData
{
    float Vergence = 0.f;
};

struct SingleEyeData : EyeData
{
    float EyeOpenness = 0.f;
    bool EyeOpenValid = false;
    float PupilDiam = false;
    FVector2D PupilPos = FVector2D::ZeroVector;
    bool PupilPosValid = false;
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
	bool LightOn = false;
	bool ButtonPressed = false;
	float gaze2target_pitch = 0.f;
	float gaze2target_yaw = 0.f;
	float head2target_pitch = 0.f;
	float head2target_yaw = 0.f;
	FVector WorldPos = FVector::ZeroVector;
	FRotator WorldRot = FRotator::ZeroRotator;
	FVector CombinedOrigin = FVector::ZeroVector; // Absolute Eye Origin
	

    // helper functions
    void Clear()
    {
        // clear all inputs to their original values
        Throttle = 0.f;
        Steering = 0.f;
        Brake = 0.f;
        ToggledReverse = false;
        HoldHandbrake = false;
    }
};

struct SensorData // everything being held by this sensor
{
    SensorData()
    {
        // Assign default values to SensorData
        Combined.Origin = FVector(-5.60, 0.14, 1.12);
        Left.Origin = FVector(-6.308, 3.247, 1.264);
        Right.Origin = FVector(-5.284, -3.269, 1.014);
        FocusActorName = FString("None");
    }
    int64_t TimestampSR = 0;    // timestamp of when the frame was captured by SRanipal. in Seconds
    int64_t TimestampCarla = 0; // Timestamp within Carla of when the EyeTracker Tick() occurred
    int64_t FrameSequence = 0;  // Frame sequence of the SRanipal
    ComboEyeData Combined;
    SingleEyeData Left, Right;
    // HMD position and orientation
    FVector HMDLocation = FVector::ZeroVector;    // initialized as {0,0,0}
    FRotator HMDRotation = FRotator::ZeroRotator; // initialized to {0,0,0}
    // Ego variables
    float Velocity;
    // FFocusInfo data
    FString FocusActorName;                        // Tag of the actor being focused on
    FVector FocusActorPoint = FVector::ZeroVector; // Hit point of the Focus Actor
    float FocusActorDist = 0.f;                    // Distance to the Focus Actor
    // User inputs
    struct UserInputs Inputs;

	// peripheral variables
	//bool TargetPresent;
	//float pitch;
	//float yaw;
};
}; // namespace DReyeVR