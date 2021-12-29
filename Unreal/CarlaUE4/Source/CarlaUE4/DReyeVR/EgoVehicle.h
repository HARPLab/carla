#pragma once

#include "Camera/CameraComponent.h"            // UCameraComponent
#include "Carla/Vehicle/CarlaWheeledVehicle.h" // ACarlaWheeledVehicle
#include "Components/AudioComponent.h"         // UAudioComponent
#include "Components/InputComponent.h"         // InputComponent
#include "Components/SceneComponent.h"         // USceneComponent
#include "CoreMinimal.h"                       // Unreal functions
#include "DReyeVRUtils.h"                      // ReadConfigValue
#include "EgoSensor.h"                         // AEgoSensor
#include "FlatHUD.h"                           // ADReyeVRHUD
#include "ImageUtils.h"                        // CreateTexture2D
#include "LevelScript.h"                       // ADReyeVRLevel
#include "WheeledVehicle.h"                    // VehicleMovementComponent
#include <stdio.h>
#include <vector>

#define USE_LOGITECH_WHEEL false

#ifndef _WIN32
// can only use LogitechWheel plugin on Windows!
#undef USE_LOGITECH_WHEEL
#define USE_LOGITECH_WHEEL false
#endif

#if USE_LOGITECH_WHEEL
#include "LogitechSteeringWheelLib.h" // LogitechWheel plugin for Force Feedback
#endif

#include "EgoVehicle.generated.h"

class ADReyeVRLevel;

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
    void SetLevel(ADReyeVRLevel *Level);

    FVector GetCameraOffset() const;

    FVector GetCameraPosn() const;
    FVector GetNextCameraPosn(const float DeltaTime) const;
    FRotator GetCameraRot() const;

    UCameraComponent *GetCamera();
    const UCameraComponent *GetCamera() const;

    const USceneComponent *GetVRCameraRoot() const;
    USceneComponent *GetVRCameraRoot();

    void PlayGearShiftSound(const float DelayBeforePlay = 0.f);
    void PlayTurnSignalSound(const float DelayBeforePlay = 0.f);

    void ReplayUpdate();

  protected:
    // Called when the game starts (spawned) or ends (destroyed)
    virtual void BeginPlay() override;
    void Register();
    virtual void BeginDestroy() override;

    // World variables
    class UWorld *World;
    class APlayerController *Player;

    // Level variables
    void TickLevel(float DeltaSeconds);
    ADReyeVRLevel *DReyeVRLevel = nullptr;

  private:
    // Camera Variables
    UPROPERTY(Category = Camera, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class USceneComponent *VRCameraRoot;
    UPROPERTY(Category = Camera, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent *FirstPersonCam;
    float FieldOfView = 90.f;

    // Camera control functions
    FVector CameraLocnInVehicle{21.0f, -40.0f, 120.0f}; // tunable per vehicle
    void InitCamera();
    void CameraPositionAdjust(const FVector &displacement);
    void CameraFwd();
    void CameraBack();
    void CameraLeft();
    void CameraRight();
    void CameraUp();
    void CameraDown();

    // Sensor Components
    class AEgoSensor *EgoSensor;       // custom sensor class that holds all our DReyeVR data
    DReyeVR::UserInputs VehicleInputs; // struct for user inputs
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

    // Vehicle parameters
    float ScaleSteeringInput;
    float ScaleThrottleInput;
    float ScaleBrakeInput;
    bool InvertMouseY;
    float ScaleMouseY;
    float ScaleMouseX;

#if USE_LOGITECH_WHEEL
    DIJOYSTATE2 *Old = nullptr; // global "old" struct for the last state
    void LogLogitechPluginStruct(const DIJOYSTATE2 *Now);
    void LogitechWheelUpdate();      // for logitech wheel integration
    void ApplyForceFeedback() const; // for logitech wheel integration
#endif

    // Cosmetic
    void InitReticleTexture();  // initializes the spectator-reticle texture
    TArray<FColor> ReticleSrc;  // pixel values array for eye reticle texture
    UTexture2D *ReticleTexture; // UE4 texture for eye reticle

    // HUD variables (NOTE: ONLY FOR NON VR)
    class ADReyeVRHUD *HUD;
    void DrawHUD(float DeltaSeconds);
    int ReticleSize = 100;               // diameter of reticle (line thickness is 10% of this)
    bool bDrawFPSCounter = true;         // draw FPS counter in top left corner
    bool bDrawGaze = false;              // whether or not to draw a line for gaze-ray on HUD
    bool bDrawSpectatorReticle = true;   // Reticle used in the VR-spectator mode
    bool bDrawFlatReticle = true;        // Reticle used in the flat mode (uses HUD) (ONLY in non-vr mode)
    bool bEnableSpectatorScreen = false; // don't spent time rendering the spectator screen
    bool bRectangularReticle = false;    // draw the reticle texture on the HUD & Spectator (NOT RECOMMENDED)

    // Text Render components (Like the HUD but works in VR)
    void InitDReyeVRText();
    void DrawSpectatorScreen();
    FVector DashboardLocnInVehicle{110, 0, 105};
    UPROPERTY(Category = Text, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UTextRenderComponent *Speedometer;
    UPROPERTY(Category = Text, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UTextRenderComponent *TurnSignals;
    float TurnSignalDuration = 3.0f; // time in seconds
    UPROPERTY(Category = Text, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UTextRenderComponent *GearShifter;
    void UpdateText();

    // Collision w/ other vehicles
    void InitDReyeVRCollisions();
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp,
                        int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

    // Audio components
    void InitDReyeVRSounds();
    // void SoundUpdate(); // handled in parent class (with Engine rev)
    class UAudioComponent *GearShiftSound;  // nice for toggling reverse
    class UAudioComponent *TurnSignalSound; // good for turn signals
    class UAudioComponent *CrashSound;      // crashing with another actor

    // Eye gaze variables
    bool bDrawDebugEditor = false;
    FVector CombinedGaze, CombinedOrigin;
    FVector LeftGaze, LeftOrigin;
    FVector RightGaze, RightOrigin;

    // Other variables
    void ReadConfigVariables();
    void InitVehicleMovement();
    bool IsHMDConnected = false;  // checks for HMD connection on BeginPlay
    bool IsLogiConnected = false; // check if Logi device is connected (on BeginPlay)
};
