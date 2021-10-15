#include "EgoVehicle.h"
#include "Carla/Vehicle/VehicleControl.h"      // FVehicleControl
#include "DrawDebugHelpers.h"                  // Debug Line/Sphere
#include "Engine/EngineTypes.h"                // EBlendMode
#include "Engine/World.h"                      // GetWorld
#include "GameFramework/Actor.h"               // Destroy
#include "HeadMountedDisplayFunctionLibrary.h" // SetTrackingOrigin, GetWorldToMetersScale
#include "HeadMountedDisplayTypes.h"           // ESpectatorScreenMode
#include "Kismet/GameplayStatics.h"            // GetPlayerController
#include "Kismet/KismetSystemLibrary.h"        // PrintString, QuitGame
#include "Math/Rotator.h"                      // RotateVector, Clamp
#include "Math/UnrealMathUtility.h"            // Clamp

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

    // Initialize collision event functions
    InitDReyeVRCollisions();

    // Initialize audio components
    InitDReyeVRSounds();

    // Initialize mirrors
    InitDReyeVRMirrors();
}

void AEgoVehicle::InitVehicleMovement()
{
    // TODO: adjust vehicle params here
    // possibly overriding changes in CARLA wheeled vehicle
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
    Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.0f, 0.0f);
    Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(7000.0f, 0.0f);
    //    Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.0f, 10.0f);
    //    Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(1890.0f, 100.0f);
    //    Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(5730.0f, 50.0f);

    // Adjust the steering
    Vehicle4W->SteeringCurve.GetRichCurve()->Reset();
    Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(0.0f, 1.0f);
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
    TurnSignals->SetRelativeLocation(DashboardLocnInVehicle + FVector(0, -40.f, 0));
    TurnSignals->SetRelativeRotation(FRotator(0.f, 180.f, 0.f)); // need to flip it to get the text in driver POV
    TurnSignals->SetTextRenderColor(FColor::Red);
    TurnSignals->SetText(FText::FromString(""));
    TurnSignals->SetXScale(1.f);
    TurnSignals->SetYScale(1.f);
    TurnSignals->SetWorldSize(10); // scale the font with this
    TurnSignals->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    TurnSignals->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);

    // Create speedometer
    GearShifter = CreateDefaultSubobject<UTextRenderComponent>(TEXT("GearShifter"));
    GearShifter->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    GearShifter->SetRelativeLocation(DashboardLocnInVehicle + FVector(0, 15.f, 0));
    GearShifter->SetRelativeRotation(FRotator(0.f, 180.f, 0.f)); // need to flip it to get the text in driver POV
    GearShifter->SetTextRenderColor(FColor::Red);
    GearShifter->SetText(FText::FromString("D"));
    GearShifter->SetXScale(1.f);
    GearShifter->SetYScale(1.f);
    GearShifter->SetWorldSize(10); // scale the font with this
    GearShifter->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    GearShifter->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
}

void AEgoVehicle::InitDReyeVRCollisions()
{
    // using Carla's GetVehicleBoundingBox function
    UE_LOG(LogTemp, Log, TEXT("Initializing collisions"));
    // AActor::GetActorBounds(false, Origin, BoxExtent); // not sure why incorrect
    // UBoxComponent *Bounds = ACarlaWheeledVehicle::GetVehicleBoundingBox();
    FVector Origin(3.07f, -0.59f, 74.74f); // obtained by looking at blueprint values
    FVector Scale3D(7.51f, 3.38f, 2.37f);  // obtained by looking at blueprint values
    FVector BoxExtent(32.f, 32.f, 32.f);   // obtained by looking at blueprint values
    // FVector BoxExtent2 = this->GetVehicleBoundingBoxExtent();
    // FTransform BoxTransform = this->GetVehicleTransform(); // this->GetVehicleBoundingBoxTransform();
    // UE_LOG(LogTemp, Log, TEXT("Detected origin %.3f %.3f %.3f"), BoxTransform.GetLocation().X,
    //        BoxTransform.GetLocation().Y, BoxTransform.GetLocation().Z);
    // UE_LOG(LogTemp, Log, TEXT("Detected scale %.3f %.3f %.3f"), BoxTransform.GetScale3D().X,
    //        BoxTransform.GetScale3D().Y, BoxTransform.GetScale3D().Z);
    // UE_LOG(LogTemp, Log, TEXT("Detected extent %.3f %.3f %.3f"), BoxExtent2.X, BoxExtent2.Y, BoxExtent2.Z);
    Bounds = CreateDefaultSubobject<UBoxComponent>(TEXT("DReyeVRBoundingBox"));
    Bounds->SetupAttachment(GetRootComponent());
    Bounds->SetBoxExtent(BoxExtent);
    Bounds->SetRelativeScale3D(Scale3D);
    Bounds->SetRelativeLocation(Origin);
    Bounds->SetGenerateOverlapEvents(true);
    Bounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Bounds->SetCollisionProfileName(TEXT("Trigger"));
    Bounds->OnComponentBeginOverlap.AddDynamic(this, &AEgoVehicle::OnOverlapBegin);
}

void AEgoVehicle::OnOverlapBegin(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                                 UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                 const FHitResult &SweepResult)
{
    if (OtherActor != nullptr && OtherActor != this)
    {
        FString actor_name = OtherActor->GetName();
        UE_LOG(LogTemp, Log, TEXT("Collision with \"%s\""), *actor_name);
        // can be more flexible, such as having collisions with static props or people too
        if (OtherActor->IsA(ACarlaWheeledVehicle::StaticClass()))
        {
            // emit the car collision sound at the midpoint between the vehicles' collision
            const FVector Location = (OtherActor->GetActorLocation() - GetActorLocation()) / 2.f;
            // const FVector Location = OtherActor->GetActorLocation(); // more pronounced spacial audio
            const FRotator Rotation(0.f, 0.f, 0.f);
            const float VolMult = 1.f; //((OtherActor->GetVelocity() - GetVelocity()) / 40.f).Size();
            const float PitchMult = 1.f;
            const float SoundStartTime = 0.f; // how far in to the sound to begin playback
            // "fire and forget" sound function
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), CrashSound->Sound, Location, Rotation, VolMult, PitchMult,
                                                  SoundStartTime, CrashSound->AttenuationSettings, nullptr, this);
        }
    }
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

    static ConstructorHelpers::FObjectFinder<USoundWave> CarCrashSound(
        TEXT("SoundWave'/Game/Carla/Blueprints/Vehicles/DReyeVR/Sounds/Crash.Crash'"));
    CrashSound = CreateDefaultSubobject<UAudioComponent>(TEXT("CarCrash"));
    CrashSound->SetupAttachment(GetRootComponent());
    CrashSound->SetSound(CarCrashSound.Object);
}

void AEgoVehicle::InitDReyeVRMirrors()
{
    static ConstructorHelpers::FObjectFinder<UMaterial> MirrorTexture(
        TEXT("Material'/Game/Carla/Blueprints/Vehicles/DReyeVR/"
             "Mirror_DReyeVR.Mirror_DReyeVR'"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneSM(TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
    RearMirror = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RearMirror"));
    RearMirror->SetStaticMesh(PlaneSM.Object);
    RearMirror->SetMaterial(0, MirrorTexture.Object);
    RearMirror->AttachTo(GetMesh());
    RearMirror->SetRelativeLocation(FVector(76.f, 0.f, 127.f));
    RearMirror->SetRelativeRotation(FRotator(90.f, 0.f, -15.f)); // Y Z X (euler angles)
    RearMirror->SetRelativeScale3D(FVector(0.13f, 0.2775f, 1.f));
    RearMirror->SetGenerateOverlapEvents(false); // don't collide with itself
    RearMirror->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    RearReflection = CreateDefaultSubobject<UPlanarReflectionComponent>(TEXT("RearReflection"));
    RearReflection->AttachTo(RearMirror);
    RearReflection->SetRelativeLocation(FVector(0.f, 0.f, 2.f));
    RearReflection->SetRelativeRotation(FRotator(5.f, 0.f, -5.f));
    RearReflection->SetRelativeScale3D(FVector(0.0625f, 0.035f, 1.f));
    RearReflection->NormalDistortionStrength = 0.0f;
    RearReflection->PrefilterRoughness = 0.0f;
    RearReflection->DistanceFromPlaneFadeoutStart = 1500.f;
    RearReflection->DistanceFromPlaneFadeoutEnd = 0.f;
    RearReflection->AngleFromPlaneFadeStart = 90.f;
    RearReflection->AngleFromPlaneFadeEnd = 90.f;
    RearReflection->PrefilterRoughnessDistance = 10000.f;
    RearReflection->ScreenPercentage = 100.f; // change this to reduce quality & improve performance
    RearReflection->bShowPreviewPlane = false;
    RearReflection->HideComponent(GetMesh());

    LeftMirror = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftMirror"));
    LeftMirror->SetStaticMesh(PlaneSM.Object);
    LeftMirror->SetMaterial(0, MirrorTexture.Object);
    LeftMirror->AttachTo(GetMesh());
    LeftMirror->SetRelativeLocation(FVector(58.f, -98.f, 104.f));
    LeftMirror->SetRelativeRotation(FRotator(90.f, 0.f, 25.4f)); // Y Z X (euler angles)
    LeftMirror->SetRelativeScale3D(FVector(0.13f, 0.2475f, 1.f));
    LeftMirror->SetGenerateOverlapEvents(false); // don't collide with itself
    LeftMirror->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    LeftReflection = CreateDefaultSubobject<UPlanarReflectionComponent>(TEXT("LeftReflection"));
    LeftReflection->AttachTo(LeftMirror);
    LeftReflection->SetRelativeLocation(FVector(0.f, -15.f, 12.2f));
    LeftReflection->SetRelativeRotation(FRotator(-5.f, 0.f, -7.f));
    LeftReflection->SetRelativeScale3D(FVector(0.035f, 0.043f, 1.f));
    LeftReflection->NormalDistortionStrength = 0.0f;
    LeftReflection->PrefilterRoughness = 0.0f;
    LeftReflection->DistanceFromPlaneFadeoutStart = 1500.f;
    LeftReflection->DistanceFromPlaneFadeoutEnd = 0.f;
    LeftReflection->AngleFromPlaneFadeStart = 90.f;
    LeftReflection->AngleFromPlaneFadeEnd = 90.f;
    LeftReflection->PrefilterRoughnessDistance = 10000.f;
    LeftReflection->ScreenPercentage = 100.f; // change this to reduce quality & improve performance
    LeftReflection->bShowPreviewPlane = false;
    LeftReflection->HideComponent(GetMesh());

    RightMirror = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightMirror"));
    RightMirror->SetStaticMesh(PlaneSM.Object);
    RightMirror->SetMaterial(0, MirrorTexture.Object);
    RightMirror->AttachTo(GetMesh());
    RightMirror->SetRelativeLocation(FVector(58.f, 98.f, 104.f));
    RightMirror->SetRelativeRotation(FRotator(90.f, 0.f, -25.4f)); // Y Z X (euler angles)
    RightMirror->SetRelativeScale3D(FVector(0.13f, 0.2475f, 1.f));
    RightMirror->SetGenerateOverlapEvents(false); // don't collide with itself
    RightMirror->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    RightReflection = CreateDefaultSubobject<UPlanarReflectionComponent>(TEXT("RightReflection"));
    RightReflection->AttachTo(RightMirror);
    RightReflection->SetRelativeLocation(FVector(11.f, -4.f, 6.13f));
    RightReflection->SetRelativeRotation(FRotator(-5.f, 0.f, -7.f));
    RightReflection->SetRelativeScale3D(FVector(0.03f, 0.05f, 1.f));
    RightReflection->NormalDistortionStrength = 0.0f;
    RightReflection->PrefilterRoughness = 0.0f;
    RightReflection->DistanceFromPlaneFadeoutStart = 1500.f;
    RightReflection->DistanceFromPlaneFadeoutEnd = 0.f;
    RightReflection->AngleFromPlaneFadeStart = 90.f;
    RightReflection->AngleFromPlaneFadeEnd = 90.f;
    RightReflection->PrefilterRoughnessDistance = 10000.f;
    RightReflection->ScreenPercentage = 100.f; // change this to reduce quality & improve performance
    RightReflection->bShowPreviewPlane = false;
    RightReflection->HideComponent(GetMesh());
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
    //    return VRCameraRoot->GetRelativeLocation();
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

    // Enable VR spectator screen & eye reticle
    InitReticleTexture(); // generate array of pixel values
    if (bDisableSpectatorScreen)
    {
        UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenMode(ESpectatorScreenMode::Disabled);
    }
    else if (DrawSpectatorReticle && IsHMDConnected)
    {
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
    DebugLines();

    // Draw text on scren (like a HUD but works in VR)
    UpdateText();

    // Draw the reticle on the Viewport (red square on the flat-screen window) while playing VR
    DrawReticle();

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
    EyeTrackerSensor->UpdateEgoVelocity(GetVehicleForwardSpeed());
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
//    DrawDebugSphere(World, CombinedGazePosn, 4.0f, 12, FColor::Blue);
//
//    // Draw individual rays for left and right eye
//    DrawDebugLine(World,
//                  LeftOrigin,                                        // start line
//                  LeftOrigin + 10 * WorldRot.RotateVector(LeftGaze), // end line
//                  FColor::Green, false, -1, 0, 1);
//
//    DrawDebugLine(World,
//                  RightOrigin,                                         // start line
//                  RightOrigin + 10 * WorldRot.RotateVector(RightGaze), // end line
//                  FColor::Yellow, false, -1, 0, 1);
#endif
    if (DrawGazeOnHUD)
    {
        // Draw line components in HUD
        HUD->DrawDynamicLine(CombinedOrigin, CombinedOrigin + 10.f * WorldRot.RotateVector(CombinedGaze), FColor::Red,
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
    const bool bRectangularReticle = false;          // TODO: parametrize
    ReticleSrc.Reserve(ReticleDim.X * ReticleDim.Y); // allocate width*height space
    for (int i = 0; i < ReticleDim.X; i++)
    {
        for (int j = 0; j < ReticleDim.Y; j++)
        {
            // RGBA colours
            FColor Colour;
            if (bRectangularReticle)
            {
                bool LeftOrRight = (i < ReticleThickness.X || i > ReticleDim.X - ReticleThickness.X);
                bool TopOrBottom = (j < ReticleThickness.Y || j > ReticleDim.Y - ReticleThickness.Y);
                if (LeftOrRight || TopOrBottom)
                    Colour = FColor(255, 0, 0, 128); // (semi-opaque red)
                else
                    Colour = FColor(0, 0, 0, 0); // (fully transparent inside)
            }
            else
            {
                const int x = i - ReticleDim.X / 2;
                const int y = j - ReticleDim.Y / 2;
                const float Radius = ReticleDim.X / 3.f;
                const int RadThickness = 3;
                const int LineLen = 4 * RadThickness;
                const float RadLo = Radius - LineLen;
                const float RadHi = Radius + LineLen;
                bool BelowRadius = (FMath::Square(x) + FMath::Square(y) <= FMath::Square(Radius + RadThickness));
                bool AboveRadius = (FMath::Square(x) + FMath::Square(y) >= FMath::Square(Radius - RadThickness));
                if (BelowRadius && AboveRadius)
                    Colour = FColor(255, 0, 0, 128); // (semi-opaque red)
                else
                {
                    // Draw little rectangular markers
                    const bool RightMarker = (RadLo < x && x < RadHi) && std::fabs(y) < RadThickness;
                    const bool LeftMarker = (RadLo < -x && -x < RadHi) && std::fabs(y) < RadThickness;
                    const bool TopMarker = (RadLo < y && y < RadHi) && std::fabs(x) < RadThickness;
                    const bool BottomMarker = (RadLo < -y && -y < RadHi) && std::fabs(x) < RadThickness;
                    if (RightMarker || LeftMarker || TopMarker || BottomMarker)
                        Colour = FColor(255, 0, 0, 128); // (semi-opaque red)
                    else
                        Colour = FColor(0, 0, 0, 0); // (fully transparent inside)
                }
            }
            ReticleSrc.Add(Colour);
        }
    }
    /// NOTE: need to create transient like this bc of a UE4 bug in release mode
    // https://forums.unrealengine.com/development-discussion/rendering/1767838-fimageutils-createtexture2d-crashes-in-packaged-build
    ReticleTexture = UTexture2D::CreateTransient(ReticleDim.X, ReticleDim.Y, PF_B8G8R8A8);
    void *TextureData = ReticleTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(TextureData, ReticleSrc.GetData(), 4 * ReticleDim.X * ReticleDim.Y);
    ReticleTexture->PlatformData->Mips[0].BulkData.Unlock();
    ReticleTexture->UpdateResource();
    // ReticleTexture = FImageUtils::CreateTexture2D(ReticleDim.X, ReticleDim.Y, ReticleSrc, GetWorld(),
    //                                               "EyeReticleTexture", EObjectFlags::RF_Transient, params);

    check(ReticleTexture);
    check(ReticleTexture->Resource);
}

void AEgoVehicle::DrawReticle()
{
    if (bDisableSpectatorScreen)
    {
        return;
    }
    const FRotator WorldRot = FirstPersonCam->GetComponentRotation();
    // 1m away from the origin
    const FVector CombinedGazePosn = CombinedOrigin + WorldRot.RotateVector(CombinedGaze);

    if (Player) // Get size of the viewport
        Player->GetViewportSize(ViewSize.X, ViewSize.Y);
    /// NOTE: this is the better way to get the ViewportSize
    const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
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
    if (IsHMDConnected)
    {
        if (DrawSpectatorReticle)
        {
            UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenModeTexturePlusEyeLayout(
                FVector2D{0.f, 0.f}, // whole window (top left)
                FVector2D{1.f, 1.f}, // whole window (top ->*bottom? right)
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
            // HUD->DrawDynamicSquare(CombinedGazePosn, 25, FColor(0, 255, 0, 255), 2);
            // HUD->DrawDynamicSquare(CombinedGazePosn, 60, FColor(255, 0, 0, 255), 5);
            if (!ensure(ReticleTexture) || !ensure(ReticleTexture->Resource))
            {
                InitReticleTexture();
            }
            if (ReticleTexture != nullptr && ReticleTexture->Resource != nullptr)
            {
                /// TODO: add scale
                HUD->DrawDynamicTexture(ReticleTexture,
                                        ReticlePos + FVector2D(-ReticleDim.X * 0.5f, -ReticleDim.Y * 0.5f));
                // see here for guide on DrawTexture
                // https://answers.unrealengine.com/questions/41214/how-do-you-use-draw-texture.html
                // HUD->DrawTextureSimple(ReticleTexture, ReticlePos.X, ReticlePos.Y, 10.f, false);
                // HUD->DrawTexture(ReticleTexture,
                //                  ReticlePos.X, // screen space X coord
                //                  ReticlePos.Y, // screen space Y coord
                //                  96,           // screen space width
                //                  96,           // screen space height
                //                  0,            // top left X of texture
                //                  0,            // top left Y of texture
                //                  1,            // bottom right X of texture
                //                  1             // bottom right Y of texture
                //                                //  FLinearColor::White,           // tint colour
                //                                //  EBlendMode::BLEND_Translucent, // blend mode
                //                                //  1.f,                           // scale
                //                                //  false,                         // scale position
                //                                //  0.f,                           // rotation
                //                                //  FVector2D::ZeroVector          // rotation pivot
                // );
            }
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
        const float MPH = GetVehicleForwardSpeed() * 0.0223694f; // FwdSpeed is in cm/s, mult by 0.0223694 to get mph
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
    float MPH;
    if (ADReyeVRSensor::GetIsReplaying())
    {
        MPH = ADReyeVRSensor::EgoReplayVelocity * 0.0223694f; // cm/s to mph
        UE_LOG(LogTemp, Log, TEXT("Velocity %.3f"), MPH);
    }
    else
        MPH = GetVehicleForwardSpeed() * 0.0223694f; // FwdSpeed is in cm/s, mult by 0.0223694 to get mph

    const FString Data = FString::FromInt(int(FMath::RoundHalfFromZero(MPH)));
    Speedometer->SetText(FText::FromString(Data));

    // Draw the signals
    float Now = FPlatformTime::Seconds();
    if (Now < RightSignalTimeToDie)
        TurnSignals->SetText(FText::FromString(">>>"));
    else if (Now < LeftSignalTimeToDie)
        TurnSignals->SetText(FText::FromString("<<<"));
    else
        TurnSignals->SetText(FText::FromString("")); // nothing

    // Draw the gear shifter
    if (bReverse)
        GearShifter->SetText(FText::FromString("R"));
    else
        GearShifter->SetText(FText::FromString("D"));
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
    PlayerInputComponent->BindAction("TurnSignalRight_DReyeVR", IE_Pressed, this, &AEgoVehicle::TurnSignalLeft);
    PlayerInputComponent->BindAction("TurnSignalLeft_DReyeVR", IE_Pressed, this, &AEgoVehicle::TurnSignalRight);
    /// Mouse X and Y input for looking up and turning
    PlayerInputComponent->BindAxis("MouseLookUp_DReyeVR", this, &AEgoVehicle::MouseLookUp);
    PlayerInputComponent->BindAxis("MouseTurn_DReyeVR", this, &AEgoVehicle::MouseTurn);
    // Record button to log the EyeTracker data to the python client
    PlayerInputComponent->BindAction("TogglePyRecord_DReyeVR", IE_Pressed, this, &AEgoVehicle::TogglePythonRecording);
    // Draw gaze rays on HUD
    PlayerInputComponent->BindAction("ToggleGazeHUD_DReyeVR", IE_Pressed, this, &AEgoVehicle::ToggleGazeHUD);
}

void AEgoVehicle::CameraPositionAdjust(const FVector &displacement)
{
    FVector CurrRelLocation = VRCameraRoot->GetRelativeLocation();
    FVector NewRelLocation = CurrRelLocation + displacement;
    VRCameraRoot->SetRelativeLocation(NewRelLocation);
}

/// NOTE: the CarlaVehicle does not actually move the vehicle, only its state/animations
// to actually move the vehicle we'll use GetVehicleMovementComponent() which is part of AWheeledVehicle
void AEgoVehicle::SetSteering(const float SteeringInput)
{
    float SteeringDamping = 0.6f;
    float ScaledSteeringInput = SteeringDamping * SteeringInput;
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

    // apply new light state
    FVehicleLightState Lights = GetVehicleLightState();
    Lights.RightBlinker = true;
    Lights.LeftBlinker = false;
    SetVehicleLightState(Lights);

    // Play turn signal sound
    if (TurnSignalSound)
    {
        const float Delay = 0.f; // Time (s) before playing sound
        TurnSignalSound->Play(Delay);
    }
    RightSignalTimeToDie = FPlatformTime::Seconds() + 3.0f; // reset counter at 3s
    LeftSignalTimeToDie = 0.f;                              // immediately stop left signal
}

void AEgoVehicle::TurnSignalLeft()
{
    // store in local input container
    VehicleInputs.TurnSignalLeft = true;

    // apply new light state
    FVehicleLightState Lights = GetVehicleLightState();
    Lights.RightBlinker = false;
    Lights.LeftBlinker = true;
    SetVehicleLightState(Lights);

    // Play turn signal sound
    if (TurnSignalSound)
    {
        const float Delay = 0.f; // Time (s) before playing sound
        TurnSignalSound->Play(Delay);
    }
    RightSignalTimeToDie = 0.f;                            // immediately stop right signal
    LeftSignalTimeToDie = FPlatformTime::Seconds() + 3.0f; // reset counter at 3s
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
    //    if (WheelState->rgdwPOV[0] == 0) // should work now
    if (WheelState->rgbButtons[0] || WheelState->rgbButtons[1] || WheelState->rgbButtons[2] ||
        WheelState->rgbButtons[3]) // replace reverse with face buttons
    {
        if (isPressRisingEdgeRev == true) // only toggle reverse on rising edge of button press
        {
            isPressRisingEdgeRev = false; // not rising edge while the button is pressed
            UE_LOG(LogTemp, Log, TEXT("Reversing: Dpad value %f"), Dpad);
            ToggleReverse();
        }
    }
    else
    {
        isPressRisingEdgeRev = true;
    }
    if (WheelState->rgbButtons[4])
    {
        TurnSignalRight();
    }
    if (WheelState->rgbButtons[5])
    {
        TurnSignalLeft();
    }

    // VRCamerRoot base position adjustment
    if (WheelState->rgdwPOV[0] == 0) // positive in X
        AEgoVehicle::CameraPositionAdjust(FVector(1.f, 0.f, 0.f));
    else if (WheelState->rgdwPOV[0] == 18000) // negative in X
        AEgoVehicle::CameraPositionAdjust(FVector(-1.f, 0.f, 0.f));
    else if (WheelState->rgdwPOV[0] == 9000) // positive in Y
        AEgoVehicle::CameraPositionAdjust(FVector(0.f, 1.f, 0.f));
    else if (WheelState->rgdwPOV[0] == 27000) // negative in Y
        AEgoVehicle::CameraPositionAdjust(FVector(0.f, -1.f, 0.f));
    // VRCamerRoot base height adjustment
    else if (WheelState->rgbButtons[19]) // positive in Z
        AEgoVehicle::CameraPositionAdjust(FVector(0.f, 0.f, 1.f));
    else if (WheelState->rgbButtons[20]) // negative in Z
        AEgoVehicle::CameraPositionAdjust(FVector(0.f, 0.f, -1.f));
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
        const int OffsetPercentage = 0;      // "Specifies the center of the spring force effect"
        const int SaturationPercentage = 30; // "Level of saturation... comparable to a magnitude"
        const int CoeffPercentage = 100; // "Slope of the effect strength increase relative to deflection from Offset"
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