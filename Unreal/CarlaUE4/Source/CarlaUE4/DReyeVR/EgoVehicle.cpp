#include "EgoVehicle.h"
#include "Carla/Vehicle/VehicleControl.h"      // FVehicleControl
#include "DrawDebugHelpers.h"                  // Debug Line/Sphere
#include "Engine/World.h"                      // GetWorld
#include "GameFramework/Actor.h"               // Destroy
#include "HeadMountedDisplayFunctionLibrary.h" // SetTrackingOrigin, GetWorldToMetersScale
#include "HeadMountedDisplayTypes.h"           // ESpectatorScreenMode
#include "Kismet/GameplayStatics.h"            // GetPlayerController
#include "Kismet/KismetSystemLibrary.h"        // PrintString, QuitGame
#include "Math/Rotator.h"                      // RotateVector, Clamp
#include "Math/UnrealMathUtility.h"            // Clamp
#include "Components/SphereComponent.h"		   // Sphere Starter Content
#include "EgoVehicleHelper.h"

#include <algorithm>

// Sets default values
AEgoVehicle::AEgoVehicle(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    // Initialize vehicle movement component
    InitVehicleMovement();

    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Set this pawn to be controlled by first (only) player
    AutoPossessPlayer = EAutoReceiveInput::Player0;

    // Set up the root position to be the this mesh
    SetRootComponent(GetMesh());

    // Initialize the camera components
    InitCamera();

    // Initialize text render components
    InitDReyeVRText();

    // Initialize audio components
    InitDReyeVRSounds();

	
}

void AEgoVehicle::InitVehicleMovement()
{
    UWheeledVehicleMovementComponent4W *Vehicle4W =
        CastChecked<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());

    // Adjust the tire loading
    Vehicle4W->MinNormalizedTireLoad = 0.0f;
    Vehicle4W->MinNormalizedTireLoadFiltered = 0.2f;
    Vehicle4W->MaxNormalizedTireLoad = 2.0f;
    Vehicle4W->MaxNormalizedTireLoadFiltered = 2.0f;

    // Torque setup
    Vehicle4W->MaxEngineRPM = 5700.0f;
    Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->Reset();
    Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.0f, 400.0f);
    Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(1890.0f, 500.0f);
    Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(5730.0f, 400.0f);

    // Adjust the steering
    Vehicle4W->SteeringCurve.GetRichCurve()->Reset();
    Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(0.0f, 1.0f);
    Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(1890.0f, 500.0f);
    Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(120.0f, 0.6f);

    Vehicle4W->DifferentialSetup.DifferentialType = EVehicleDifferential4W::LimitedSlip_4W;
    Vehicle4W->DifferentialSetup.FrontRearSplit = 0.65f;

    // Automatic gearbox
    Vehicle4W->TransmissionSetup.bUseGearAutoBox = true;
    Vehicle4W->TransmissionSetup.GearSwitchTime = 0.15f;
    Vehicle4W->TransmissionSetup.GearAutoBoxLatency = 1.0f;

    /// TODO: adjust the deceleration curve
}

void AEgoVehicle::InitCamera()
{
    // Spawn the RootComponent and Camera for the VR camera
    VRCameraRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRCameraRoot"));
    VRCameraRoot->SetupAttachment(GetRootComponent());      // The vehicle blueprint itself
    VRCameraRoot->SetRelativeLocation(CameraLocnInVehicle); // Offset from center of camera

    // Create a camera and attach to root component
    FirstPersonCam = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCam"));
    FirstPersonCam->SetupAttachment(VRCameraRoot);
    FirstPersonCam->bUsePawnControlRotation = false; // free for VR movement
    FirstPersonCam->FieldOfView = 90.0f;             // editable
}

void AEgoVehicle::InitDReyeVRText()
{
    // Create speedometer
    Speedometer = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Speedometer"));
    Speedometer->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    Speedometer->SetRelativeLocation(DashboardLocnInVehicle);
    Speedometer->SetRelativeRotation(FRotator(0.f, 180.f, 0.f)); // need to flip it to get the text in driver POV
    Speedometer->SetTextRenderColor(FColor::Red);
    Speedometer->SetText(FText::FromString("0"));
    Speedometer->SetXScale(1.f);
    Speedometer->SetYScale(1.f);
    Speedometer->SetWorldSize(10); // scale the font with this
    Speedometer->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    Speedometer->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);

    // Create turn signals
    TurnSignals = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TurnSignals"));
    TurnSignals->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    TurnSignals->SetRelativeLocation(DashboardLocnInVehicle + FVector(0, -40, 0));
    TurnSignals->SetRelativeRotation(FRotator(0.f, 180.f, 0.f)); // need to flip it to get the text in driver POV
    TurnSignals->SetTextRenderColor(FColor::Red);
    TurnSignals->SetText(FText::FromString(""));
    TurnSignals->SetXScale(1.f);
    TurnSignals->SetYScale(1.f);
    TurnSignals->SetWorldSize(10); // scale the font with this
    TurnSignals->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    TurnSignals->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
}

void AEgoVehicle::InitDReyeVRSounds()
{
    // retarget the engine cue
    static ConstructorHelpers::FObjectFinder<USoundCue> EgoEngineCue(
        TEXT("SoundCue'/Game/Carla/Blueprints/Vehicles/DReyeVR/Sounds/EgoEngineRev.EgoEngineRev'"));
    EngineRevSound->SetSound(EgoEngineCue.Object); // using this sound

    // Initialize audio components
    static ConstructorHelpers::FObjectFinder<USoundWave> GearSound(
        TEXT("SoundWave'/Game/Carla/Blueprints/Vehicles/DReyeVR/Sounds/GearShift.GearShift'"));
    GearShiftSound = CreateDefaultSubobject<UAudioComponent>(TEXT("GearShift"));
    GearShiftSound->SetupAttachment(GetRootComponent());
    // GearShiftSound->Activate(true);
    GearShiftSound->SetSound(GearSound.Object);

    static ConstructorHelpers::FObjectFinder<USoundWave> TurnSignalSoundWave(
        TEXT("SoundWave'/Game/Carla/Blueprints/Vehicles/DReyeVR/Sounds/TurnSignal.TurnSignal'"));
    TurnSignalSound = CreateDefaultSubobject<UAudioComponent>(TEXT("TurnSignal"));
    TurnSignalSound->SetupAttachment(GetRootComponent());
    // TurnSignalSound->Activate(true);
    TurnSignalSound->SetSound(TurnSignalSoundWave.Object);
}

void AEgoVehicle::ErrMsg(const FString &message, const bool isFatal = false)
{
    /// NOTE: solely for debugging
    UKismetSystemLibrary::PrintString(World, message, true, true, FLinearColor(1, 0, 0, 1), 20.0f);
    if (isFatal)
        UKismetSystemLibrary::QuitGame(World, Player, EQuitPreference::Quit, false);
    return;
}

FVector AEgoVehicle::GetFPSPosn() const
{
    return FirstPersonCam->GetComponentLocation();
}

FRotator AEgoVehicle::GetFPSRot() const
{
    return FirstPersonCam->GetComponentRotation();
}

FVector AEgoVehicle::GetCameraOffset() const
{
    return CameraLocnInVehicle;
}

void AEgoVehicle::BeginPlay()
{
    // Called when the game starts or when spawned
    Super::BeginPlay();

    // Get information about the world
    World = GetWorld();
    Player = UGameplayStatics::GetPlayerController(World, 0); // main player (0) controller

    // Setup the HUD
    AHUD *Raw_HUD = Player->GetHUD();
    HUD = CastChecked<ADReyeVRHUD>(Raw_HUD);
    HUD->SetPlayer(Player);

    // Get information about the VR headset
    IsHMDConnected = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
    if (IsHMDConnected)
    {
        UE_LOG(LogTemp, Log, TEXT("HMD detected"));
        // Now we'll begin with setting up the VR Origin logic
        UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye); // Also have Floor & Stage Level
        FirstPersonCam->SetRelativeLocation(HMDOffset);                                 // Offset Camera
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("NO HMD detected"));
    }

    // Spawn the EyeTracker Carla sensor and attach to Ego-Vehicle:
    FActorSpawnParameters EyeTrackerSpawnInfo;
    EyeTrackerSpawnInfo.Owner = this;
    EyeTrackerSpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    EyeTrackerSensor = World->SpawnActor<AEyeTracker>(FirstPersonCam->GetComponentLocation(),
                                                      FRotator(0.0f, 0.0f, 0.0f), EyeTrackerSpawnInfo);
    // Attach the EyeTrackerSensor as a child to the EgoVehicle BP
    EyeTrackerSensor->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
    EyeTrackerSensor->SetPlayer(Player);
    EyeTrackerSensor->SetCamera(FirstPersonCam);


	// Spawn LightBall (George)
	FActorSpawnParameters LightBallSpawnInfo;
	LightBallSpawnInfo.Owner = this;
	LightBallSpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	LightBallObject = World->SpawnActor<ALightBall>(FirstPersonCam->GetComponentLocation(),
		FRotator(0.0f, 0.0f, 0.0f), LightBallSpawnInfo);
	// Turn off collision/transparent
	LightBallObject->SetActorEnableCollision(false);










    // Draw the Eye reticle on the screen
    if (DrawSpectatorReticle && IsHMDConnected)
    {
        InitReticleTexture(); // generate array of pixel values
        /// NOTE: need to create transient like this bc of a UE4 bug in release mode
        // https://forums.unrealengine.com/development-discussion/rendering/1767838-fimageutils-createtexture2d-crashes-in-packaged-build
        ReticleTexture = UTexture2D::CreateTransient(ReticleDim.X, ReticleDim.Y, PF_B8G8R8A8);
        void *TextureData = ReticleTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
        FMemory::Memcpy(TextureData, ReticleSrc.GetData(), 4 * ReticleDim.X * ReticleDim.Y);
        ReticleTexture->PlatformData->Mips[0].BulkData.Unlock();
        ReticleTexture->UpdateResource();
        // ReticleTexture = FImageUtils::CreateTexture2D(ReticleDim.X, ReticleDim.Y, ReticleSrc, GetWorld(),
        //                                               "EyeReticleTexture", EObjectFlags::RF_Transient, params);
        UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenMode(ESpectatorScreenMode::TexturePlusEye);
        UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenTexture(ReticleTexture);
    }

    // Initialize logitech steering wheel
#if USE_LOGITECH_WHEEL
    LogiSteeringInitialize(false);
    IsLogiConnected = LogiIsConnected(0); // get status of connected device
    if (!IsLogiConnected)
    {
        UE_LOG(LogTemp, Log, TEXT("WARNING: Could not find Logitech device connected on input 0"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Found a Logitech device connected on input 0"));
    }
#endif

    // Bug-workaround for initial delay on throttle; see https://github.com/carla-simulator/carla/issues/1640
    FVehicleControl ManualControl;
    ManualControl.bManualGearShift = true;
    ManualControl.Gear = 1;
    ApplyVehicleControl(ManualControl, EVehicleInputPriority::User);
    // then switch back to automatic
    FVehicleControl AutomaticControl;
    AutomaticControl.bManualGearShift = false;
    ApplyVehicleControl(AutomaticControl, EVehicleInputPriority::User);

    UE_LOG(LogTemp, Log, TEXT("Initialized DReyeVR EgoVehicle"));
}

void AEgoVehicle::BeginDestroy()
{
    // destroy all spawned entities
    if (EyeTrackerSensor)
        EyeTrackerSensor->Destroy();

    Super::BeginDestroy();
}

void AEgoVehicle::ReplayUpdate()
{
    // perform all updates that occur when replaying
    if (ADReyeVRSensor::GetIsReplaying())
    {
        // if we don't have this positional update here, there is lag/jitter between the FPS camera and the vehicle
        // since they are likely updating on different ticks (Carla Replayers's vs here)
        SetActorTransform(ADReyeVRSensor::EgoReplayTransform, false, nullptr, ETeleportType::None);
    }
}

// Called every frame
void AEgoVehicle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update the positions based off replay data
    ReplayUpdate();

    // Update sensor
    UpdateSensor(DeltaTime);

    // Draw debug lines on editor
	// DebugLines();

    // Draw text on scren (like a HUD but works in VR)
    UpdateText();

    // Draw the reticle on the Viewport (red square on the flat-screen window) while playing VR
    DrawReticle();

	// Draw stimuli
	const FRotator WorldRot = FirstPersonCam->GetComponentRotation();
	const FVector WorldPos = FirstPersonCam->GetComponentLocation();
	FVector HeadDirection = WorldRot.Vector();
	FVector CombinedGazePosn = CombinedOrigin + WorldRot.RotateVector(CombinedGaze);
	GenerateSphere(HeadDirection, CombinedGazePosn, WorldRot, WorldPos, LightBallObject, DeltaTime);

	/*
	UE_LOG(LogTemp, Log, TEXT("CombinedGazePosn logging %s"), *CombinedGazePosn.ToString());
	UE_LOG(LogTemp, Log, TEXT("CombinedOrigin logging %s"), *CombinedOrigin.ToString());
	UE_LOG(LogTemp, Log, TEXT("CombinedGaze logging %s"), *CombinedGaze.ToString());
	*/
	UE_LOG(LogTemp, Log, TEXT("ButtonPress, %d"), VehicleInputs.ButtonPressed);

#if USE_LOGITECH_WHEEL
    if (IsLogiConnected)
    {
        // Taking logitech inputs for steering
        LogitechWheelUpdate();

        // Add Force Feedback to the hardware steering wheel when a LogitechWheel is used
        ApplyForceFeedback();
    }
#endif
}

/// ========================================== ///
/// ----------------:SENSOR:------------------ ///
/// ========================================== ///

void AEgoVehicle::UpdateSensor(const float DeltaSeconds)
{
    // Compute World positions and orientations
    EyeTrackerSensor->SetInputs(VehicleInputs);
    EyeTrackerSensor->Tick(DeltaSeconds);
    // clear inputs to be updated on the next tick
    VehicleInputs.Clear();

    // Calculate gaze data
    // Compute World positions and orientations
    const FRotator WorldRot = FirstPersonCam->GetComponentRotation();
    const FVector WorldPos = FirstPersonCam->GetComponentLocation();

    // First get the gaze origin and direction and vergence from the EyeTracker Sensor
    const float Vergence = EyeTrackerSensor->GetVergence(); // vergence already in m
    // scaling gaze ray to world units
    const float UE4MeterScale = UHeadMountedDisplayFunctionLibrary::GetWorldToMetersScale(World);

    // Both eyes
    CombinedGaze = Vergence * UE4MeterScale * EyeTrackerSensor->GetCenterGazeRay();
    CombinedOrigin = WorldRot.RotateVector(EyeTrackerSensor->GetCenterOrigin()) + WorldPos;

    // Left eye
    LeftGaze = UE4MeterScale * EyeTrackerSensor->GetLeftGazeRay();
    LeftOrigin = WorldRot.RotateVector(EyeTrackerSensor->GetLeftOrigin()) + WorldPos;

    // Right eye
    RightGaze = UE4MeterScale * EyeTrackerSensor->GetRightGazeRay();
    RightOrigin = WorldRot.RotateVector(EyeTrackerSensor->GetRightOrigin()) + WorldPos;
}

/// ========================================== ///
/// ---------------:COSMETIC:----------------- ///
/// ========================================== ///

void AEgoVehicle::DebugLines() const
{
    // Compute World positions and orientations
    const FRotator WorldRot = FirstPersonCam->GetComponentRotation();

#if WITH_EDITOR
    // Rotate and add the gaze ray to the origin
    FVector CombinedGazePosn = CombinedOrigin + WorldRot.RotateVector(CombinedGaze);

    // Use Absolute Ray Position to draw debug information
    DrawDebugSphere(World, CombinedGazePosn, 4.0f, 12, FColor::Blue);

    // Draw individual rays for left and right eye
    DrawDebugLine(World,
                  LeftOrigin,                                        // start line
                  LeftOrigin + 10 * WorldRot.RotateVector(LeftGaze), // end line
                  FColor::Green, false, -1, 0, 1);

    DrawDebugLine(World,
                  RightOrigin,                                         // start line
                  RightOrigin + 10 * WorldRot.RotateVector(RightGaze), // end line
                  FColor::Yellow, false, -1, 0, 1);
#endif
    if (DrawGazeOnHUD)
    {
        // Draw line components in HUD
        HUD->DrawDynamicLine(CombinedOrigin, CombinedOrigin + 10 * WorldRot.RotateVector(CombinedGaze), FColor::Red,
                             3.0f);
    }
}

void AEgoVehicle::ToggleGazeHUD()
{
    UE_LOG(LogTemp, Log, TEXT("Toggling gaze on HUD"));
    DrawGazeOnHUD = !DrawGazeOnHUD;
}

void AEgoVehicle::InitReticleTexture()
{
    // Used to initialize any bitmap-based image that will be used as a reticle
    // draw a box with line thickness 2px
    ReticleSrc.Reserve(ReticleDim.X * ReticleDim.Y); // allocate width*height space
    for (int i = 0; i < ReticleDim.X; i++)
    {
        for (int j = 0; j < ReticleDim.Y; j++)
        {
            // RGBA colours
            bool LeftOrRight = (i < ReticleThickness.X || i > ReticleDim.X - ReticleThickness.X);
            bool TopOrBottom = (j < ReticleThickness.Y || j > ReticleDim.Y - ReticleThickness.Y);
            if (LeftOrRight || TopOrBottom)
                ReticleSrc.Add(FColor(255, 0, 0, 128)); // (semi-opaque red)
            else
                ReticleSrc.Add(FColor(0, 0, 0, 0)); // (fully transparent inside)
        }
    }
}

void AEgoVehicle::DrawReticle()
{
    const FRotator WorldRot = FirstPersonCam->GetComponentRotation();
    // 1m away from the origin
    const FVector CombinedGazePosn = CombinedOrigin + WorldRot.RotateVector(CombinedGaze);
    if (IsHMDConnected)
    {
        if (DrawSpectatorReticle)
        {
            if (Player) // Get size of the viewport
                Player->GetViewportSize(ViewSize.X, ViewSize.Y);
            FVector2D ReticlePos;
            if (Player)
                UGameplayStatics::ProjectWorldToScreen(Player, CombinedGazePosn, ReticlePos, true);
            /// NOTE: the SetSpectatorScreenModeTexturePlusEyeLayout expects normalized positions on the screen
            /// NOTE: to get the best drawing, the texture is offset slightly by this vector
            const FVector2D ScreenOffset(ReticleDim.X * 0.5f,
                                         -ReticleDim.Y); // move X right by Dim.X/2, move Y up by Dim.Y
            /// TODO: clamp to be within [0,1]
            FVector2D TextureRectMin((ReticlePos.X + ScreenOffset.X) / ViewSize.X,
                                     (ReticlePos.Y + ScreenOffset.Y) / ViewSize.Y);
            // max needs to define the bottom right corner, so needs to be +Dim.X right, and +Dim.Y down
            FVector2D TextureRectMax((ReticlePos.X + ScreenOffset.X + ReticleDim.X) / ViewSize.X,
                                     (ReticlePos.Y + ScreenOffset.Y + ReticleDim.Y) / ViewSize.Y);
            UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenModeTexturePlusEyeLayout(
                FVector2D{0.f, 0.f}, // whole window (top left)
                FVector2D{1.f, 1.f}, // whole window (top right)
                TextureRectMin,      // top left of texture
                TextureRectMax,      // bottom right of texture
                true,                // draw eye data as background
                false,               // clear w/ black
                true                 // use alpha
            );
        }
    }
    else
    {
        if (DrawFlatReticle)
        {
            // Draw on user HUD (only for flat-view)
            HUD->DrawDynamicSquare(CombinedGazePosn, 25, FColor(0, 255, 0, 255), 2);
            HUD->DrawDynamicSquare(CombinedGazePosn, 60, FColor(255, 0, 0, 255), 5);
        }
    }
}

// void AEgoVehicle::SoundUpdate()
// {
//     // overwrite from parent class (ACarlaWheeledVehicle)
//     if (EngineRevSound)
//     {
//         float RPM = FMath::Clamp(GetVehicleMovement()->GetEngineRotationSpeed(), 0.f, 5650.0f);
//         EngineRevSound->SetFloatParameter(FName("RPM"), RPM);
//     }
// }

void AEgoVehicle::DrawHUD()
{
    if (!IsHMDConnected) // flat view
    {
        /// NOTE: this only really works in a non-vr setting!!
        // Draw text components
        const float MPH = GetVehicleForwardSpeed() * 0.0223694; // FwdSpeed is in cm/s, mult by 0.0223694 to get mph
        FString Data = FString::FromInt(int(FMath::RoundHalfFromZero(MPH)));
        // found this position via the BP editor
        const FVector DashboardPosn = GetActorLocation() + GetActorRotation().RotateVector(FVector(120, 0, 105));
        HUD->DrawDynamicText(Data, DashboardPosn, FColor(255, 0, 0, 255), 4, false);

        // Draw the signals
        float Now = FPlatformTime::Seconds();
        const FVector DashCenter = GetActorLocation() + GetActorRotation().RotateVector(FVector(120, -40, 110));
        if (Now < RightSignalTimeToDie)
        {
            HUD->DrawDynamicText("<", DashCenter, FColor(255, 0, 0, 255), 10, false);
        }
        if (Now < LeftSignalTimeToDie)
        {
            HUD->DrawDynamicText(">", DashCenter, FColor(255, 0, 0, 255), 10, false);
        }
    }
}

void AEgoVehicle::UpdateText()
{
    // Draw text components
    const float MPH = GetVehicleForwardSpeed() * 0.0223694; // FwdSpeed is in cm/s, mult by 0.0223694 to get mph
    FString Data = FString::FromInt(int(FMath::RoundHalfFromZero(MPH)));
    Speedometer->SetText(FText::FromString(Data));

    // Draw the signals
    float Now = FPlatformTime::Seconds();
    if (Now < RightSignalTimeToDie)
        TurnSignals->SetText(FText::FromString(">>>"));
    else if (Now < LeftSignalTimeToDie)
        TurnSignals->SetText(FText::FromString("<<<"));
    else
        TurnSignals->SetText(FText::FromString("")); // nothing
}

/// ========================================== ///
/// ----------------:INPUTS:------------------ ///
/// ========================================== ///

// Called to bind functionality to input
void AEgoVehicle::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    /// NOTE: to see all DReyeVR inputs see DefaultInput.ini
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    /// NOTE: an Action is a digital input, an Axis is an analog input
    // steering and throttle analog inputs (axes)
    PlayerInputComponent->BindAxis("Steer_DReyeVR", this, &AEgoVehicle::SetSteering);
    PlayerInputComponent->BindAxis("Throttle_DReyeVR", this, &AEgoVehicle::SetThrottle);
    PlayerInputComponent->BindAxis("Brake_DReyeVR", this, &AEgoVehicle::SetBrake);
    // reverse & handbrake actions
    PlayerInputComponent->BindAction("ToggleReverse_DReyeVR", IE_Pressed, this, &AEgoVehicle::ToggleReverse);
    PlayerInputComponent->BindAction("HoldHandbrake_DReyeVR", IE_Pressed, this, &AEgoVehicle::HoldHandbrake);
    PlayerInputComponent->BindAction("HoldHandbrake_DReyeVR", IE_Released, this, &AEgoVehicle::ReleaseHandbrake);

    PlayerInputComponent->BindAction("TurnSignalRight_DReyeVR", IE_Pressed, this, &AEgoVehicle::PressButton);
    PlayerInputComponent->BindAction("TurnSignalLeft_DReyeVR", IE_Pressed, this, &AEgoVehicle::PressButton);
	PlayerInputComponent->BindAction("TurnSignalRight_DReyeVR", IE_Released, this, &AEgoVehicle::ReleaseButton);
	PlayerInputComponent->BindAction("TurnSignalLeft_DReyeVR", IE_Released, this, &AEgoVehicle::ReleaseButton);

	// peripheral response button
	//PlayerInputComponent->BindAction("PeripheralResponseButton", IE_Pressed, this, &AEgoVehicle::PeripheralResponseButtonPressed);
	//PlayerInputComponent->BindAction("PeripheralResponseButton", IE_Released, this, &AEgoVehicle::PeripheralResponseButtonReleased);

    /// Mouse X and Y input for looking up and turning
    PlayerInputComponent->BindAxis("MouseLookUp_DReyeVR", this, &AEgoVehicle::MouseLookUp);
    PlayerInputComponent->BindAxis("MouseTurn_DReyeVR", this, &AEgoVehicle::MouseTurn);
    // Record button to log the EyeTracker data to the python client
    PlayerInputComponent->BindAction("TogglePyRecord_DReyeVR", IE_Pressed, this, &AEgoVehicle::TogglePythonRecording);
    // Draw gaze rays on HUD
    PlayerInputComponent->BindAction("ToggleGazeHUD_DReyeVR", IE_Pressed, this, &AEgoVehicle::ToggleGazeHUD);
}


/// NOTE: the CarlaVehicle does not actually move the vehicle, only its state/animations
// to actually move the vehicle we'll use GetVehicleMovementComponent() which is part of AWheeledVehicle
void AEgoVehicle::SetSteering(const float SteeringInput)
{
    float ScaledSteeringInput = 1.0f * SteeringInput;
    GetVehicleMovementComponent()->SetSteeringInput(ScaledSteeringInput); // UE4 control
    SetSteeringInput(ScaledSteeringInput);                                // Carla control
    // assign to input struct
    VehicleInputs.Steering = ScaledSteeringInput;
}

void AEgoVehicle::SetThrottle(const float ThrottleInput)
{
    float ScaledThrottleInput = 1.0f * ThrottleInput;
    GetVehicleMovementComponent()->SetThrottleInput(ScaledThrottleInput); // UE4 control
    SetThrottleInput(ScaledThrottleInput);                                // Carla control

    // apply new light state
    FVehicleLightState Lights = GetVehicleLightState();
    Lights.Reverse = false;
    Lights.Brake = false;
    SetVehicleLightState(Lights);

    // assign to input struct
    VehicleInputs.Throttle = ScaledThrottleInput;
}

void AEgoVehicle::SetBrake(const float BrakeInput)
{
    float ScaledBrakeInput = 2.0f * BrakeInput;
    GetVehicleMovementComponent()->SetBrakeInput(ScaledBrakeInput); // UE4 control
    SetBrakeInput(ScaledBrakeInput);                                // Carla control

    // apply new light state
    FVehicleLightState Lights = GetVehicleLightState();
    Lights.Reverse = false;
    Lights.Brake = true;
    SetVehicleLightState(Lights);

    // assign to input struct
    VehicleInputs.Brake = ScaledBrakeInput;
}

void AEgoVehicle::ToggleReverse()
{
    // negate to toggle bw 1 (forwards) and -1 (backwards)
    int NewGear = -1 * GetVehicleMovementComponent()->GetTargetGear();
    bReverse = !bReverse;
    GetVehicleMovementComponent()->SetTargetGear(NewGear, true); // UE4 control
    SetReverse(bReverse);                                        // Carla control

    // apply new light state
    FVehicleLightState Lights = GetVehicleLightState();
    Lights.Reverse = bReverse;
    SetVehicleLightState(Lights);

    UE_LOG(LogTemp, Log, TEXT("Toggle Reverse"));
    // assign to input struct
    VehicleInputs.ToggledReverse = true;
    // Play gear shift sound
    if (GearShiftSound)
    {
        const float Delay = 0.f; // Time (s) before playing sound
        GearShiftSound->Play(Delay);
    }
}

void AEgoVehicle::TurnSignalRight()
{
    // store in local input container
    VehicleInputs.TurnSignalRight = true;
	VehicleInputs.ButtonPressed = true;

    // apply new light state
    //FVehicleLightState Lights = GetVehicleLightState();
    //Lights.RightBlinker = true;
    //Lights.LeftBlinker = false;
    //SetVehicleLightState(Lights);

    // Play turn signal sound
    if (TurnSignalSound)
    {
        const float Delay = 0.f; // Time (s) before playing sound
        TurnSignalSound->Play(Delay);
    }
    //RightSignalTimeToDie = FPlatformTime::Seconds() + 3.0f; // reset counter at 3s
	RightSignalTimeToDie = 0.f;
    LeftSignalTimeToDie = 0.f;                              // immediately stop left signal
}

void AEgoVehicle::TurnSignalLeft()
{
    // store in local input container
    VehicleInputs.TurnSignalLeft = true;
	VehicleInputs.ButtonPressed = true;

    // apply new light state
    //FVehicleLightState Lights = GetVehicleLightState();
    //Lights.RightBlinker = false;
    //Lights.LeftBlinker = true;
    //SetVehicleLightState(Lights);

    // Play turn signal sound
    if (TurnSignalSound)
    {
        const float Delay = 0.f; // Time (s) before playing sound
        TurnSignalSound->Play(Delay);
    }
    RightSignalTimeToDie = 0.f;                            // immediately stop right signal
    //LeftSignalTimeToDie = FPlatformTime::Seconds() + 3.0f; // reset counter at 3s
	LeftSignalTimeToDie = 0.f;
}

void AEgoVehicle::PressButton()
{
	VehicleInputs.ButtonPressed = true;

	// Play turn signal sound
	if (TurnSignalSound)
	{
		const float Delay = 0.f; // Time (s) before playing sound
		TurnSignalSound->Play(Delay);
	}
}

void AEgoVehicle::ReleaseButton()
{
	VehicleInputs.ButtonPressed = false;
}

void AEgoVehicle::HoldHandbrake()
{
    GetVehicleMovementComponent()->SetHandbrakeInput(true); // UE4 control
    SetHandbrakeInput(true);                                // Carla control
    // assign to input struct
    VehicleInputs.HoldHandbrake = true;
}

void AEgoVehicle::ReleaseHandbrake()
{
    GetVehicleMovementComponent()->SetHandbrakeInput(false); // UE4 control
    SetHandbrakeInput(false);                                // Carla control
    // assign to input struct
    VehicleInputs.HoldHandbrake = false;
}

/// NOTE: in UE4 rotators are of the form: {Pitch, Yaw, Roll} (stored in degrees)
/// We are basing the limits off of "Cervical Spine Functional Anatomy ad the Biomechanics of Injury":
// "The cervical spine's range of motion is approximately 80° to 90° of flexion, 70° of extension,
// 20° to 45° of lateral flexion, and up to 90° of rotation to both sides."
// (www.ncbi.nlm.nih.gov/pmc/articles/PMC1250253/)
/// NOTE: flexion = looking down to chest, extension = looking up , lateral = roll
/// ALSO: These functions are only used in non-VR mode, in VR you can move freely

void AEgoVehicle::MouseLookUp(const float mY_Input)
{
    if (mY_Input != 0.f)
    {
        const int ScaleY = InvertY ? 1 : -1; // negative Y is "normal" controls
        FRotator UpDir = FirstPersonCam->GetRelativeRotation() + FRotator(ScaleY * mY_Input, 0.f, 0.f);
        // get the limits of a human neck (only clamping pitch)
        const float MinFlexion = -85.f;
        const float MaxExtension = 70.f;
        UpDir.Pitch = FMath::Clamp(UpDir.Pitch, MinFlexion, MaxExtension);
        FirstPersonCam->SetRelativeRotation(UpDir);
    }
}

void AEgoVehicle::MouseTurn(const float mX_Input)
{
    if (mX_Input != 0.f)
    {
        FRotator CurrentDir = FirstPersonCam->GetRelativeRotation();
        FRotator TurnDir = CurrentDir + FRotator(0.f, mX_Input, 0.f);
        // get the limits of a human neck (only clamping pitch)
        const float MinLeft = -90.f;
        const float MaxRight = 90.f; // may consider increasing to allow users to look through the back window
        TurnDir.Yaw = FMath::Clamp(TurnDir.Yaw, MinLeft, MaxRight);
        FirstPersonCam->SetRelativeRotation(TurnDir);
    }
}

void AEgoVehicle::GenerateSphere(FVector HeadDirection, FVector CombinedGazePosn, FRotator WorldRot,
	FVector CombinedOrigin, ALightBall *LightBallObject, float DeltaTime)
{
	float CenterMagnitude = (CombinedGazePosn - CombinedOrigin).Size();
	FVector UnitGazeVec = (CombinedGazePosn - CombinedOrigin) / CenterMagnitude;
	//UE_LOG(LogTemp, Log, TEXT("UnitGazVec, %s"), *UnitGazeVec.ToString());

	// generate stimuli every 5 second chunks, and log that time
	if (TimeSinceIntervalStart < 5) {
		if (TimeSinceIntervalStart == 0.f) {
			// Generate light posn wrt head direction
			RotVec = GenerateRotVec(HeadDirection, yawMax, pitchMax, vert_offset);

			// Get angles between head direction and light posn
			auto angles = AEgoVehicle::GetAngles(HeadDirection, RotVec);
			curr_pitch = std::get<0>(angles);
			curr_yaw = std::get<1>(angles);

			// Generate random time to start flashing during the interval
			TimeStart = FMath::RandRange(1.f, 4.f);
			TimeSinceIntervalStart += DeltaTime;

		} 
		else if (FMath::IsNearlyEqual(TimeSinceIntervalStart, TimeStart, 0.05f)) {
			// turn light on 
			LightBallObject->TurnLightOn();
			TimeSinceIntervalStart += DeltaTime;
			//UE_LOG(LogTemp, Log, TEXT("Light On"));
			VehicleInputs.LightOn = true;
		}
		else if (FMath::IsNearlyEqual(TimeSinceIntervalStart, TimeStart + FlashDuration, 0.05f)) {
			// turn light off
			LightBallObject->TurnLightOff();
			TimeSinceIntervalStart += DeltaTime;
			//UE_LOG(LogTemp, Log, TEXT("Light Off"));
			VehicleInputs.LightOn = false;
		}
		else {
			TimeSinceIntervalStart += DeltaTime;
		}
	}
	else {
		TimeSinceIntervalStart = 0.f;
	}

	//UE_LOG(LogTemp, Log, TEXT("DeltaTime, %f"), DeltaTime);
	//UE_LOG(LogTemp, Log, TEXT("TimeSinceIntervalStart, %f"), TimeSinceIntervalStart);
	
	// Generate the posn of the light given current angles
	float DistanceFromDriver = CenterMagnitude*3;
	FVector RotVecDirection = GenerateRotVecGivenAngles(HeadDirection, curr_yaw, curr_pitch);
	FVector RotVecDirectionPosn = CombinedOrigin + RotVecDirection * DistanceFromDriver;
	LightBallObject->SetLocation(RotVecDirectionPosn);

	/*
	UE_LOG(LogTemp, Log, TEXT("RotVec, %s"), *RotVec.ToString());
	UE_LOG(LogTemp, Log, TEXT("RotVecDirection, %s"), *RotVecDirection.ToString());
	*/

	// Calculate gaze angles of light posn wrt eye gaze
	auto gaze_angles = AEgoVehicle::GetAngles(UnitGazeVec, RotVecDirection);
	gaze_pitch = std::get<0>(gaze_angles);
	gaze_yaw = std::get<1>(gaze_angles);

	UE_LOG(LogTemp, Log, TEXT("curr_pitch, %f"), curr_pitch);
	UE_LOG(LogTemp, Log, TEXT("curr_yaw, %f"), curr_yaw);
	UE_LOG(LogTemp, Log, TEXT("gaze_pitch, %f"), gaze_pitch);
	UE_LOG(LogTemp, Log, TEXT("gaze_yaw, %f"), gaze_yaw);
	
	// Draw debug border markers
	FVector TopLeftLimit = GenerateRotVecGivenAngles(UnitGazeVec, -yawMax, pitchMax+vert_offset) * DistanceFromDriver;
	FVector TopRightLimit = GenerateRotVecGivenAngles(UnitGazeVec, yawMax, pitchMax+vert_offset) * DistanceFromDriver;
	FVector BotLeftLimit = GenerateRotVecGivenAngles(UnitGazeVec, -yawMax, -pitchMax+vert_offset) * DistanceFromDriver;
	FVector BotRightLimit = GenerateRotVecGivenAngles(UnitGazeVec, yawMax, -pitchMax+vert_offset) * DistanceFromDriver;
	DrawDebugSphere(World, CombinedOrigin + TopLeftLimit, 4.0f, 12, FColor::Blue);
	DrawDebugSphere(World, CombinedOrigin + TopRightLimit, 4.0f, 12, FColor::Blue);
	DrawDebugSphere(World, CombinedOrigin + BotLeftLimit, 4.0f, 12, FColor::Blue);
	DrawDebugSphere(World, CombinedOrigin + BotRightLimit, 4.0f, 12, FColor::Blue);
}


void AEgoVehicle::TogglePythonRecording()
{
    bool FoundSensor = false;
    if (IsRecording)
    {
        UE_LOG(LogTemp, Log, TEXT("Ego-Vehicle stops logging"));
        if (EyeTrackerSensor)
            FoundSensor = EyeTrackerSensor->ResetPyDReyeVRSensor();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Ego-Vehicle starts logging"));
        if (EyeTrackerSensor)
            FoundSensor = EyeTrackerSensor->FindPyDReyeVRSensor();
    }
    if (FoundSensor) // at least one is found in the Simulator (spawned)
        IsRecording = !IsRecording;
}

void AEgoVehicle::SetVolume(const float Mult)
{
    if (GearShiftSound)
    {
        GearShiftSound->SetVolumeMultiplier(Mult);
    }
    if (TurnSignalSound)
    {
        TurnSignalSound->SetVolumeMultiplier(Mult);
    }
    Super::SetVolume(Mult); // mutes engine rev
}

#if USE_LOGITECH_WHEEL

const std::vector<FString> VarNames = {"rgdwPOV[0]", "rgdwPOV[1]", "rgdwPOV[2]", "rgdwPOV[3]"};
void AEgoVehicle::LogLogitechPluginStruct(const DIJOYSTATE2 *Now)
{
    if (Old == nullptr)
    {
        Old = new struct DIJOYSTATE2;
        (*Old) = (*Now); // assign to the new (current) dijoystate struct
        return;          // initializing the Old struct ptr
    }
    // Getting all (4) values from the current struct
    const std::vector<int> NowVals = {int(Now->rgdwPOV[0]), int(Now->rgdwPOV[1]), int(Now->rgdwPOV[2]),
                                      int(Now->rgdwPOV[3])};
    // Getting the (4) values from the old struct
    const std::vector<int> OldVals = {int(Old->rgdwPOV[0]), int(Old->rgdwPOV[1]), int(Old->rgdwPOV[2]),
                                      int(Old->rgdwPOV[3])};

    check(NowVals.size() == OldVals.size() && NowVals.size() == VarNames.size());

    // print any differences
    bool isDiff = false;
    for (size_t i = 0; i < NowVals.size(); i++)
    {
        if (NowVals[i] != OldVals[i])
        {
            if (!isDiff) // only gets triggered at MOST once
            {
                UE_LOG(LogTemp, Log, TEXT("Logging joystick at t=%.3f"), UGameplayStatics::GetRealTimeSeconds(World));
                isDiff = true;
            }
            UE_LOG(LogTemp, Log, TEXT("Triggered \"%s\" from %d to %d"), *(VarNames[i]), OldVals[i], NowVals[i]);
        }
    }

    // also check the 128 rgbButtons array
    for (size_t i = 0; i < 127; i++)
    {
        if (Old->rgbButtons[i] != Now->rgbButtons[i])
        {
            if (!isDiff) // only gets triggered at MOST once
            {
                UE_LOG(LogTemp, Log, TEXT("Logging joystick at t=%.3f"), UGameplayStatics::GetRealTimeSeconds(World));
                isDiff = true;
            }
            UE_LOG(LogTemp, Log, TEXT("Triggered \"rgbButtons[%d]\" from %d to %d"), int(i), int(OldVals[i]),
                   int(NowVals[i]));
        }
    }

    // assign the current joystate into the old one
    (*Old) = (*Now);
}

void AEgoVehicle::LogitechWheelUpdate()
{
    // only execute this in Windows, the Logitech plugin is incompatible with Linux
    LogiUpdate(); // update the logitech wheel
    DIJOYSTATE2 *WheelState = LogiGetState(0);
    LogLogitechPluginStruct(WheelState);
    /// NOTE: obtained these from LogitechWheelInputDevice.cpp:~111
    // -32768 to 32767. -32768 = all the way to the left. 32767 = all the way to the right.
    const float WheelRotation = FMath::Clamp(float(WheelState->lX), -32767.0f, 32767.0f) / 32767.0f; // (-1, 1)
    // -32768 to 32767. 32767 = pedal not pressed. -32768 = pedal fully pressed.
    const float AccelerationPedal = fabs(((WheelState->lY - 32767.0f) / (65535.0f))); // (0, 1)
    // -32768 to 32767. Higher value = less pressure on brake pedal
    const float BrakePedal = fabs(((WheelState->lRz - 32767.0f) / (65535.0f))); // (0, 1)
    // -1 = not pressed. 0 = Top. 0.25 = Right. 0.5 = Bottom. 0.75 = Left.
    const float Dpad = fabs(((WheelState->rgdwPOV[0] - 32767.0f) / (65535.0f)));
    // apply to DReyeVR inputs
    SetSteering(WheelRotation);
    SetThrottle(AccelerationPedal);
    SetBrake(BrakePedal);

    //    UE_LOG(LogTemp, Log, TEXT("Dpad value %f"), Dpad);
    if (WheelState->rgdwPOV[0] == 0) // should work now
    {
        UE_LOG(LogTemp, Log, TEXT("Reversing: Dpad value %f"), Dpad);
        ToggleReverse();
    }
    if (WheelState->rgbButtons[4])
    {
		PressButton();
    }
    if (WheelState->rgbButtons[5])
    {
        PressButton();
    }
}

void AEgoVehicle::ApplyForceFeedback() const
{
    // only execute this in Windows, the Logitech plugin is incompatible with Linux
    const float Speed = GetVelocity().Size(); // get magnitude of self (AActor's) velocity
                                              //    UE_LOG(LogTemp, Log, TEXT("Speed value %f"), Speed);
    const int WheelIndex = 0;                 // first (only) wheel attached
    /// TODO: move outside this function (in tick()) to avoid redundancy
    if (LogiHasForceFeedback(WheelIndex))
    {
        const int OffsetPercentage = 0;             // "Specifies the center of the spring force effect"
        const int SaturationPercentage = 1 * Speed; // "Level of saturation... comparable to a magnitude"
        const int CoeffPercentage = 90; // "Slope of the effect strength increase relative to deflection from Offset"
        LogiPlaySpringForce(WheelIndex, OffsetPercentage, SaturationPercentage, CoeffPercentage);
    }
    /// NOTE: there are other kinds of forces as described in the LogitechWheelPlugin API:
    // https://github.com/drb1992/LogitechWheelPlugin/blob/master/LogitechWheelPlugin/Source/LogitechWheelPlugin/Private/LogitechBWheelInputDevice.cpp
    // For example:
    /*
        Force Types
        0 = Spring				5 = Dirt Road
        1 = Constant			6 = Bumpy Road
        2 = Damper				7 = Slippery Road
        3 = Side Collision		8 = Surface Effect
        4 = Frontal Collision	9 = Car Airborne
    */
}
#endif

