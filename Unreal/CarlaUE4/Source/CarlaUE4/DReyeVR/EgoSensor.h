#pragma once

#include "Carla/Sensor/DReyeVRSensor.h"         // DReyeVRSensor
#include "Carla/Sensor/DReyeVRSensorData.h"     // DReyeVRSensorData
#include "Components/SceneCaptureComponent2D.h" // USceneCaptureComponent2D
#include "CoreMinimal.h"
#include <chrono> // timing threads
#include <cstdint>

#define SRANIPAL_EYE_SWAP_FIXED false
#define USE_SRANIPAL false

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

#include "EgoSensor.generated.h"

UCLASS()
class CARLAUE4_API AEgoSensor : public ADReyeVRSensor
{
    GENERATED_BODY()

  public:
    AEgoSensor(const FObjectInitializer &ObjectInitializer);

    void PrePhysTick(float DeltaSeconds);

    // getters from EgoVehicle
    void SetPlayer(APlayerController *P);
    void SetCamera(UCameraComponent *FPSCamInEgoVehicle);
    void SetInputs(const DReyeVR::UserInputs &inputs);
    void SetEgoVelocity(const float Velocity);

    // Methods to update internal data structs
    void TickEyeTracker(); // tick hardware sensor
    void ComputeTraceFocusInfo(const ECollisionChannel TraceChannel, float TraceRadius = 0.f);
    float ComputeVergence(const FVector &L0, const FVector &LDir, const FVector &R0, const FVector &RDir) const;

  protected:
    void BeginPlay();
    void BeginDestroy();

    UWorld *World;                          // to get info about the world: time, frames, etc.
    APlayerController *Player;              // for APlayerCameraManager
    class UCameraComponent *FirstPersonCam; // for moving the camera upon recording

  private:
    int64_t TickCount = 0;    // how many ticks have been executed
    int64_t TimestampRef = 0; // reference timestamp (ms) since the hmd started ticking
    // Local instances of DReyeVR::SensorData fields for internal use and eventual copying
    DReyeVR::SRanipalData EyeSensorData;
    DReyeVR::UserInputs InputData;
    DReyeVR::FocusInfo FocusInfoData;
    float EgoVelocity; // Ego velocity is tracked bc it is hard to replay accurately with a variable timestamp

    // Eye Tracker Variables
#if USE_SRANIPAL
    SRanipalEye_Core *SRanipal;               // SRanipalEye_Core.h
    SRanipalEye_Framework *SRanipalFramework; // SRanipalEye_Framework.h
    ViveSR::anipal::Eye::EyeData *EyeData;    // SRanipal_Eyes_Enums.h
#endif

    // Frame capture
    class UTextureRenderTarget2D *CaptureRenderTarget = nullptr;
    class USceneCaptureComponent2D *FrameCap = nullptr;
    FString FrameCapLocation; // relative to game dir
    FString FrameCapFilename; // gets ${tick}.png suffix
    bool bCaptureFrameData;
    int FrameCapWidth;
    int FrameCapHeight;

    // Other variables
    std::chrono::time_point<std::chrono::system_clock> StartTime;
};