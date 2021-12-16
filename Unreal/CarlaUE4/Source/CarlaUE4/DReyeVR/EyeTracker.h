#pragma once

#include "Carla/Sensor/DReyeVRSensor.h"     // DReyeVRSensor
#include "Carla/Sensor/DReyeVRSensorData.h" // DReyeVRSensorData
#include "Components/SceneCaptureComponent2D.h"
#include "CoreMinimal.h"
#include "DReyeVRUtils.h"
#include <chrono> // timing threads
#include <cstdint>

#define SRANIPAL_EYE_SWAP_FIXED false
#define USE_SRANIPAL true

#ifndef _WIN32
// can only use SRanipal plugin on Windows!
#undef USE_SRANIPAL
#define USE_SRANIPAL false
#endif

#if USE_SRANIPAL
/// NOTE: Can only use SRanipal on Windows machines
#include "SRanipalEye.h"      // SRanipal Module Framework
#include "SRanipalEye_Core.h" // SRanipal Eye Tracker
// for some reason the SRanipal developers created an enum called "ERROR" in SRanipal/Public/SRanipal_Enums.h:28
// which is used in SRanipal/Private/SRanipal_Enums.cpp:50 & 62. However, it appears that Carla has its own #define for
// ERROR which then makes the compiler complain about multiple constants. The simplest *workaround* for this is to
// rename the ERROR in the above files to something like SR_ERROR or anything but a commonly used #define
#include "SRanipalEye_Framework.h"
#endif

#include "EyeTracker.generated.h"

UCLASS()
class CARLAUE4_API AEyeTracker : public AActor
{
    GENERATED_BODY()

  public:
    AEyeTracker();

    void Tick(float DeltaSeconds);
    int64_t TickCount = 0;

    // Getters of low level Eye Tracking data
    FVector GetCenterGazeRay() const;
    FVector GetCenterOrigin() const;
    float GetVergence() const;
    FVector GetLeftGazeRay() const;
    FVector GetLeftOrigin() const;
    FVector GetRightGazeRay() const;
    FVector GetRightOrigin() const;

    // DReyeVR sensor class instance, spawned in BeginPlay()
    ADReyeVRSensor *cppDReyeVRSensor; // Spawned from this class, not used with Python
    // ROS bridge (DReyeVR) sensor registered with Carla to interact with ROS bridge
    UPROPERTY(VisibleAnywhere)       // public to be accessed from PythonAPI (client) -> ROSbridge -> ROS
    ADReyeVRSensor *pyDReyeVRSensor; // Spawned from PythonAPI client, searched for and updated in cpp
    bool FindPyDReyeVRSensor();      // initialize the (python) DReyeVR sensor to some actor (ASensor)
    bool ResetPyDReyeVRSensor();     // reset (python) DReyeVR sensor back to a nullptr

    // getters from EgoVehicle
    void SetPlayer(APlayerController *P);
    void SetCamera(UCameraComponent *FPSCamInEgoVehicle);
    void SetInputs(const DReyeVR::UserInputs &inputs);
    void UpdateEgoVelocity(const float Velocity);

  protected:
    void BeginPlay();
    void BeginDestroy();

    UWorld *World;                          // to get info about the world: time, frames, etc.
    APlayerController *Player;              // for APlayerCameraManager
    class UCameraComponent *FirstPersonCam; // for moving the camera upon recording

  private:
    // Eye Tracker Variables
#if USE_SRANIPAL
    SRanipalEye_Core *SRanipal;               // SRanipalEye_Core.h
    SRanipalEye_Framework *SRanipalFramework; // SRanipalEye_Framework.h
    ViveSR::anipal::Eye::EyeData EyeData;     // SRanipal_Eyes_Enums.h
#endif
    int64_t TimestampRef;               // reference timestamp (ms) since the hmd started ticking
    DReyeVR::SRanipalData TickSensor(); // tick hardware sensor (should be const function?)

    // everything stored in the sensor is held in this struct
    struct DReyeVR::SensorData *SensorData;

    // Ego velocity is tracked bc it is hard to reprouce with a variable timestamp
    float EgoVelocity = 0.f;

    // Frame cap helpers
    class UTextureRenderTarget2D *CaptureRenderTarget = nullptr;
    class USceneCaptureComponent2D *FrameCap = nullptr;
    FString FrameCapLocation; // relative to game dir
    FString FrameCapFilename; // gets ${tick}.png suffix
    bool bCaptureFrameData;
    int FrameCapWidth;
    int FrameCapHeight;

    // Other variables
    std::chrono::time_point<std::chrono::system_clock> StartTime;

    // Helper functions
    bool ComputeTraceFocusInfo(const ECollisionChannel TraceChannel, // reimplementing the SRanipal Focus
                               DReyeVR::FocusInfo &F, const float radius);
    float CalculateVergenceFromDirections() const; // Calculating Vergence
};