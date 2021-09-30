#pragma once

#include "Camera/CameraComponent.h"            // UCameraComponent
#include "Carla/Vehicle/CarlaWheeledVehicle.h" // Steering, Throttle, Brake, etc.
#include "Components/AudioComponent.h"         // UAudioComponent
#include "Components/InputComponent.h"         // InputComponent
#include "Components/SceneComponent.h"         // USceneComponent
#include "CoreMinimal.h"                       // Unreal functions
#include "DReyeVRHUD.h"                        // ADReyeVRHUD
#include "EyeTracker.h"                        // AEyeTracker
#include "ImageUtils.h"                        // CreateTexture2D
#include "WheeledVehicle.h"                    // VehicleMovementComponent
#include "LightBall.h"						   // ALightBall
#include "EgoVehicleHelper.h"
#include <stdio.h>
#include <vector>
#include <tuple>

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

	// new variables added by George
	float pitchMax = 0.25;
	float yawMax = 0.6;
	float FlashDuration = 0.5f;
	float TimeBetweenFlash = 2.0f;
	float TimeSinceIntervalStart = 0.f;
	float TimeStart = 0.f;
	float curr_pitch = 0.f;
	float curr_yaw = 0.f;
	float gaze_pitch = 0.f;
	float gaze_yaw = 0.f;
	float vert_offset = 0.15f;
	int Ticks = 0;
	FVector RotVec = FVector(1, 0, 0);
	FVector DiffVec = FVector(0, 0, 0);

	float RunningTime = 0.f;

	// define the func
	/*
	Input
	---
	CombinedGazePosn: Position of the an object directly ahead of the driver's eye gaze
	CombinedOrigin: Position of driver's eyes
	LightBallObject: The class that contains properties of the flashing light
	DeltaTime: Time between each frame

	Output
	---

	*/
	void GenerateSphere(FVector HeadDirection, FVector CombinedGazePosn, FRotator WorldRot,
		FVector CombinedOrigin, ALightBall *LightBallObject, float DeltaTime);

	/*
	Input
	---
	UnitGazeVec: Unit Vector directly ahead of the driver's eye gaze
	RotVec: rotated unit vector pointing to position of flashing light

	Output
	---

	*/
	//void GetAngles(FVector UnitGazeVec, FVector RotVec);

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
    const FVector CameraLocnInVehicle{21.0f, -40.0f, 120.0f}; // tunable per vehicle
    FVector HMDOffset;                                        // tunable per all humans, default to zero-vector
    void InitCamera();

    // Sensor Components
    UPROPERTY(Category = DReyeVR, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class AEyeTracker *EyeTrackerSensor; // custom sensor class that holds all our data
    DReyeVR::UserInputs VehicleInputs;   // struct for user inputs
    void UpdateSensor(const float DeltaTime);
    void DebugLines() const;


	// Light Ball Component
	UPROPERTY(Category = DReyeVR, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class ALightBall *LightBallObject;

	// added by George (same as EgoVehicleHelper)
	std::tuple<float, float> GetAngles(FVector UnitGazeVec, FVector RotVec);
	void PeripheralResponseButtonPressed();
	void PeripheralResponseButtonReleased();
	FVector GenerateRotVec(FVector UnitGazeVec, float yawMax, float pitchMax, float vert_offset);
	FVector GenerateRotVecGivenAngles(FVector UnitGazeVec, float yaw, float pitch);

    // Vehicle Control Functions
    void SetSteering(const float SteeringInput);
    void SetThrottle(const float ThrottleInput);
    void SetBrake(const float BrakeInput);
    bool bReverse;
    void ToggleReverse();
    float RightSignalTimeToDie, LeftSignalTimeToDie; // how long the blinkers last
    void TurnSignalLeft();
    void TurnSignalRight();
	void PressButton(); // added by George
	void ReleaseButton(); // added by George
    void HoldHandbrake();
    void ReleaseHandbrake();
    void MouseLookUp(const float mY_Input);
    void MouseTurn(const float mX_Input);
#if USE_LOGITECH_WHEEL
    DIJOYSTATE2 *Old = nullptr; // global "old" struct for the last state
    void LogLogitechPluginStruct(const DIJOYSTATE2 *Now);
    void LogitechWheelUpdate();      // for logitech wheel integration
    void ApplyForceFeedback() const; // for logitech wheel integration
#endif

    // Cosmetic
    void DrawReticle();                     // on Tick(), draw new reticle in eye-gaze posn
    void InitReticleTexture();              // initializes the spectator-reticle texture
    const FVector2D ReticleThickness{8, 8}; // 8px horizontal line and 8px vertical line
    const FIntPoint ReticleDim{96, 96};     // size (px) of reticle texture (x, y)
    TArray<FColor> ReticleSrc;              // pixel values array for eye reticle texture
    UTexture2D *ReticleTexture;             // UE4 texture for eye reticle
    FIntPoint ViewSize{1920, 1080};         // Size of the open window (viewport) not resolution (default to 1080p)

    // HUD variables (NOTE: ONLY FOR NON VR)
    class ADReyeVRHUD *HUD;
    void DrawHUD();

    // Text Render components (Like the HUD but works in VR)
    void InitDReyeVRText();
    const FVector DashboardLocnInVehicle{110, 0, 105};
    UPROPERTY(Category = Text, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UTextRenderComponent *Speedometer;
    UPROPERTY(Category = Text, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UTextRenderComponent *TurnSignals;
    void UpdateText();

    // Audio components
    void InitDReyeVRSounds();
    // void SoundUpdate(); // handled in parent class (with Engine rev)
    void SetVolume(const float Mult) override; // allow us to mute all our extra sounds (gear/signals)
    class UAudioComponent *GearShiftSound;     // nice for toggling reverse
    class UAudioComponent *TurnSignalSound;    // good for turn signals

    // Eye gaze variables
    FVector CombinedGaze, CombinedOrigin;
    FVector LeftGaze, LeftOrigin;
    FVector RightGaze, RightOrigin;

    // Other variables
    void InitVehicleMovement();
    /// TODO: present these to a user
    bool IsHMDConnected = false;      // checks for HMD connection on BeginPlay
    bool InvertY = false;             // whether or not to invert the mouse-camera movement
    bool DrawGazeOnHUD = false;       // whether or not to draw a line for gaze-ray on HUD
    bool DrawSpectatorReticle = true; // Reticle used in the VR-spectator mode
    bool DrawFlatReticle = true;      // Reticle used in the flat mode (uses HUD) (ONLY in non-vr mode)
    bool IsLogiConnected = false;     // check if Logi device is connected (on BeginPlay)
};
