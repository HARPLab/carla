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
#include "LightBall.h"						                // ALightBall
#include "EgoVehicleHelper.h"                     // EgoVehicleHelper
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
	float head2light_pitch = 0.f;
	float head2light_yaw = 0.f;
	float eye2light_pitch = 0.f;
	float eye2light_yaw = 0.f;
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
	void GenerateSphere(const FVector &HeadDirection, const FVector &CombinedGazePosn, 
                      const FRotator &WorldRot, const FVector &CombinedOrigin, ALightBall *LightBallObject, float DeltaTime);

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
    void ChangePixelDensity(const float NewDensity) const;

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


	// Light Ball Component
	UPROPERTY(Category = DReyeVR, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class ALightBall *LightBallObject;

	// added by George (same as EgoVehicleHelper)
	std::tuple<float, float> GetAngles(const FVector &UnitGazeVec, const FVector &RotVec);
	void PeripheralResponseButtonPressed();
	void PeripheralResponseButtonReleased();
	FVector GenerateRotVec(const FVector &UnitGazeVec, float yawMax, float pitchMax, float vert_offset);
	FVector GenerateRotVecGivenAngles(const FVector &UnitGazeVec, float yaw, float pitch);

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

    // Mirrors
    void InitDReyeVRMirrors();
    class Mirror
    {
      public:
        bool Enabled = false;
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
