#pragma once

#include "Camera/CameraComponent.h"               // UCameraComponent
#include "Carla/Vehicle/CarlaWheeledVehicle.h"    // Steering, Throttle, Brake, etc.
#include "Components/AudioComponent.h"            // UAudioComponent
#include "Components/InputComponent.h"            // InputComponent
#include "Components/PlanarReflectionComponent.h" // Planar Reflection
#include "Components/SceneComponent.h"            // USceneComponent
#include "CoreMinimal.h"                          // Unreal functions
#include "DReyeVRHUD.h"                           // ADReyeVRHUD
#include "DReyeVRUtils.h"                         // ReadConfigValue
#include "EyeTracker.h"                           // AEyeTracker
#include "ImageUtils.h"                           // CreateTexture2D
#include "WheeledVehicle.h"                       // VehicleMovementComponent
#include <stdio.h>
#include <vector>

#define USE_LOGITECH_WHEEL true

#ifndef _WIN32
// can only use LogitechWheel plugin on Windows!
#undef USE_LOGITECH_WHEEL
#define USE_LOGITECH_WHEEL false
#endif

#if USE_LOGITECH_WHEEL
#include "LogitechSteeringWheelLib.h" // LogitechWheel plugin for Force Feedback
#endif

#include "EgoVehicle.generated.h"

UCLASS()
class CARLAUE4_API AEgoVehicle : public ACarlaWheeledVehicle
{
    GENERATED_BODY()

  public:
    // Sets default values for this pawn's properties
    AEgoVehicle(const FObjectInitializer &ObjectInitializer);

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

    FVector GetCameraOffset() const;

    FVector GetFPSPosn() const;
    FRotator GetFPSRot() const;

    void ReplayUpdate();
    void ToggleGazeHUD();

  protected:
    // Called when the game starts (spawned) or ends (destroyed)
    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;

    // World variables
    UWorld *World;
    APlayerController *Player;

    // For debug purposes
    void ErrMsg(const FString &message, const bool isFatal);

    // Start recording the data from EyeTrackerSensor
    void TogglePythonRecording();
    bool IsRecording = false;

  private:
    // Camera Variables
    UPROPERTY(Category = Camera, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class USceneComponent *VRCameraRoot;
    UPROPERTY(Category = Camera, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent *FirstPersonCam;
    float FieldOfView = 90.f;
    float PixelDensity = 1.f;

    FVector CameraLocnInVehicle{21.0f, -40.0f, 120.0f}; // tunable per vehicle
    void InitCamera();
    void CameraPositionAdjust(const FVector &displacement);

    // Sensor Components
    UPROPERTY(Category = DReyeVR, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class AEyeTracker *EyeTrackerSensor; // custom sensor class that holds all our data
    DReyeVR::UserInputs VehicleInputs;   // struct for user inputs
    void UpdateSensor(const float DeltaTime);
    void DebugLines() const;

    // Vehicle Control Functions
    void SetSteering(const float SteeringInput);
    void SetThrottle(const float ThrottleInput);
    void SetBrake(const float BrakeInput);
    bool bReverse;
    bool isPressRisingEdgeRev = true; // first press is a rising edge
    void ToggleReverse();
    float RightSignalTimeToDie, LeftSignalTimeToDie; // how long the blinkers last
    void TurnSignalLeft();
    void TurnSignalRight();
    void HoldHandbrake();
    void ReleaseHandbrake();
    void MouseLookUp(const float mY_Input);
    void MouseTurn(const float mX_Input);

    // Vehicle Control Possession
    enum Driver
    {
        HUMAN,
        AI,
        NONE,
    } CurrentDriver;
    void HandoffToHuman();
    void HandoffToAI();
    void HandoffToNone();
    void DriverHandoff(const Driver NewDriver);

#if USE_LOGITECH_WHEEL
    DIJOYSTATE2 *Old = nullptr; // global "old" struct for the last state
    void LogLogitechPluginStruct(const DIJOYSTATE2 *Now);
    void LogitechWheelUpdate();      // for logitech wheel integration
    void ApplyForceFeedback() const; // for logitech wheel integration
#endif

    // Mirrors
    void InitDReyeVRMirrors();
    class Mirror
    {
      public:
        bool Enabled;
        UPROPERTY(Category = Mirrors, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
        class UStaticMeshComponent *MirrorSM;
        UPROPERTY(Category = Mirrors, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
        class UPlanarReflectionComponent *Reflection;
        FVector MirrorPos, MirrorScale, ReflectionPos, ReflectionScale;
        FRotator MirrorRot, ReflectionRot;
        float ScreenPercentage;
        FString Name;
        friend class EgoVehicle;
    };
    void InitializeMirror(Mirror &M, UMaterial *MirrorTexture, UStaticMesh *SM);
    Mirror RightMirror, LeftMirror, RearMirror;

    // Cosmetic
    bool bDisableSpectatorScreen = false; // don't spent time rendering the spectator screen
    bool bRectangularReticle = false;     // Draw a simple box reticle instead of crosshairs
    void DrawReticle();                   // on Tick(), draw new reticle in eye-gaze posn
    void InitReticleTexture();            // initializes the spectator-reticle texture
    FVector2D ReticleThickness{8, 8};     // horizontal line and vertical line
    FIntPoint ReticleDim{96, 96};         // size (px) of reticle texture (x, y)
    TArray<FColor> ReticleSrc;            // pixel values array for eye reticle texture
    UTexture2D *ReticleTexture;           // UE4 texture for eye reticle
    FIntPoint ViewSize;                   // Size of the open window (viewport) not resolution (default to 1080p)

    // HUD variables (NOTE: ONLY FOR NON VR)
    class ADReyeVRHUD *HUD;
    void DrawHUD();

    // Text Render components (Like the HUD but works in VR)
    void InitDReyeVRText();
    FVector DashboardLocnInVehicle{110, 0, 105};
    UPROPERTY(Category = Text, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UTextRenderComponent *Speedometer;
    UPROPERTY(Category = Text, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UTextRenderComponent *TurnSignals;
    UPROPERTY(Category = Text, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UTextRenderComponent *GearShifter;
    void UpdateText();

    // Collision w/ other vehicles
    FVector BBOrigin{3.07f, -0.59f, 74.74f}; // obtained by looking at blueprint values
    FVector BBScale3D{7.51f, 3.38f, 2.37f};  // obtained by looking at blueprint values
    FVector BBBoxExtent{32.f, 32.f, 32.f};   // obtained by looking at blueprint values
    void InitDReyeVRCollisions();
    UPROPERTY(Category = BoundingBox, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    UBoxComponent *Bounds;
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp,
                        int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

    // Audio components
    void InitDReyeVRSounds();
    // void SoundUpdate(); // handled in parent class (with Engine rev)
    void SetVolume(const float Mult) override; // allow us to mute all our extra sounds (gear/signals)
    class UAudioComponent *GearShiftSound;     // nice for toggling reverse
    class UAudioComponent *TurnSignalSound;    // good for turn signals
    class UAudioComponent *CrashSound;         // crashing with another actor

    // Eye gaze variables
    bool bDrawDebugEditor = false;
    FVector CombinedGaze, CombinedOrigin;
    FVector LeftGaze, LeftOrigin;
    FVector RightGaze, RightOrigin;

    // Other variables
    void ReadConfigVariables();
    void InitVehicleMovement();
    bool IsHMDConnected = false;      // checks for HMD connection on BeginPlay
    bool InvertY = false;             // whether or not to invert the mouse-camera movement
    bool DrawGazeOnHUD = false;       // whether or not to draw a line for gaze-ray on HUD
    bool DrawSpectatorReticle = true; // Reticle used in the VR-spectator mode
    bool DrawFlatReticle = true;      // Reticle used in the flat mode (uses HUD) (ONLY in non-vr mode)
    bool IsLogiConnected = false;     // check if Logi device is connected (on BeginPlay)
};
