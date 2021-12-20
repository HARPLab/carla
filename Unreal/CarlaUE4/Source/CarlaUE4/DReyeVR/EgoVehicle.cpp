#include "EgoVehicle.h"
#include "Carla/Actor/ActorRegistry.h"              // Register
#include "Carla/Game/CarlaStatics.h"                // GetEpisode
#include "Carla/Vehicle/CarlaWheeledVehicleState.h" // ECarlaWheeledVehicleState
#include "Carla/Vehicle/VehicleControl.h"           // FVehicleControl
#include "DrawDebugHelpers.h"                       // Debug Line/Sphere
#include "Engine/EngineTypes.h"                     // EBlendMode
#include "Engine/World.h"                           // GetWorld
#include "GameFramework/Actor.h"                    // Destroy
#include "HeadMountedDisplayFunctionLibrary.h"      // SetTrackingOrigin, GetWorldToMetersScale
#include "HeadMountedDisplayTypes.h"                // ESpectatorScreenMode
#include "Kismet/GameplayStatics.h"                 // GetPlayerController
#include "Kismet/KismetSystemLibrary.h"             // PrintString, QuitGame
#include "Math/Rotator.h"                           // RotateVector, Clamp
#include "Math/UnrealMathUtility.h"                 // Clamp

#include <algorithm>

// Sets default values
AEgoVehicle::AEgoVehicle(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    ReadConfigVariables();

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

void AEgoVehicle::ReadConfigVariables()
{
    ReadDReyeVRConfig();
    ReadConfigValue("EgoVehicle", "CameraInit", CameraLocnInVehicle);
    ReadConfigValue("EgoVehicle", "DashLocation", DashboardLocnInVehicle);
    // vr
    ReadConfigValue("EgoVehicle", "FieldOfView", FieldOfView);
    ReadConfigValue("EgoVehicle", "PixelDensity", PixelDensity);
    // bounding box
    ReadConfigValue("EgoVehicle", "BBOrigin", BBOrigin);
    ReadConfigValue("EgoVehicle", "BBScale", BBScale3D);
    ReadConfigValue("EgoVehicle", "BBExtent", BBBoxExtent);
    // mirrors
    auto InitMirrorParams = [](const FString &Name, Mirror &M) {
        M.Name = Name;
        ReadConfigValue("EgoVehicle", Name + "MirrorEnabled", M.Enabled);
        ReadConfigValue("EgoVehicle", Name + "MirrorPos", M.MirrorPos);
        ReadConfigValue("EgoVehicle", Name + "MirrorScale", M.MirrorScale);
        ReadConfigValue("EgoVehicle", Name + "MirrorRot", M.MirrorRot);
        ReadConfigValue("EgoVehicle", Name + "ReflectionPos", M.ReflectionPos);
        ReadConfigValue("EgoVehicle", Name + "ReflectionScale", M.ReflectionScale);
        ReadConfigValue("EgoVehicle", Name + "ReflectionRot", M.ReflectionRot);
        ReadConfigValue("EgoVehicle", Name + "ScreenPercentage", M.ScreenPercentage);
    };
    InitMirrorParams("Right", RightMirror);
    InitMirrorParams("Left", LeftMirror);
    InitMirrorParams("Rear", RearMirror);
    // cosmetic
    ReadConfigValue("EgoVehicle", "UseRectangularReticle", bRectangularReticle);
    ReadConfigValue("EgoVehicle", "ReticleThicknessX", ReticleThickness.X);
    ReadConfigValue("EgoVehicle", "ReticleThicknessY", ReticleThickness.Y);
    ReadConfigValue("EgoVehicle", "ReticleDimX", ReticleDim.X);
    ReadConfigValue("EgoVehicle", "ReticleDimY", ReticleDim.Y);
    ReadConfigValue("EgoVehicle", "DrawDebugEditor", bDrawDebugEditor);
    ReadConfigValue("EgoVehicle", "InvertY", InvertY);
    ReadConfigValue("EgoVehicle", "DrawSpectatorReticle", DrawSpectatorReticle);
    ReadConfigValue("EgoVehicle", "DrawFlatReticle", DrawFlatReticle);
    ReadConfigValue("EgoVehicle", "DrawGazeOnHUD", DrawGazeOnHUD);
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
    FirstPersonCam->FieldOfView = FieldOfView;       // editable
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
    // FVector BoxExtent2 = this->GetVehicleBoundingBoxExtent();
    // FTransform BoxTransform = this->GetVehicleTransform(); // this->GetVehicleBoundingBoxTransform();
    // UE_LOG(LogTemp, Log, TEXT("Detected origin %.3f %.3f %.3f"), BoxTransform.GetLocation().X,
    //        BoxTransform.GetLocation().Y, BoxTransform.GetLocation().Z);
    // UE_LOG(LogTemp, Log, TEXT("Detected scale %.3f %.3f %.3f"), BoxTransform.GetScale3D().X,
    //        BoxTransform.GetScale3D().Y, BoxTransform.GetScale3D().Z);
    // UE_LOG(LogTemp, Log, TEXT("Detected extent %.3f %.3f %.3f"), BoxExtent2.X, BoxExtent2.Y, BoxExtent2.Z);
    Bounds = CreateDefaultSubobject<UBoxComponent>(TEXT("DReyeVRBoundingBox"));
    Bounds->SetupAttachment(GetRootComponent());
    Bounds->SetBoxExtent(BBBoxExtent);
    Bounds->SetRelativeScale3D(BBScale3D);
    Bounds->SetRelativeLocation(BBOrigin);
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

void AEgoVehicle::PlayGearShiftSound(const float DelayBeforePlay)
{
    if (this->GearShiftSound)
    {
        GearShiftSound->Play(DelayBeforePlay);
    }
}

void AEgoVehicle::PlayTurnSignalSound(const float DelayBeforePlay)
{
    if (this->TurnSignalSound)
    {
        this->TurnSignalSound->Play(DelayBeforePlay);
    }
}

void AEgoVehicle::InitializeMirror(Mirror &M, UMaterial *MirrorTexture, UStaticMesh *SM)
{
    if (!M.Enabled)
    {
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("Initializing %s mirror"), *M.Name)
    M.MirrorSM = CreateDefaultSubobject<UStaticMeshComponent>(FName(*(M.Name + "MirrorSM")));
    M.MirrorSM->SetStaticMesh(SM);
    M.MirrorSM->SetMaterial(0, MirrorTexture);
    M.MirrorSM->AttachTo(GetMesh());
    M.MirrorSM->SetRelativeLocation(M.MirrorPos);
    M.MirrorSM->SetRelativeRotation(M.MirrorRot); // Y Z X (euler angles)
    M.MirrorSM->SetRelativeScale3D(M.MirrorScale);
    M.MirrorSM->SetGenerateOverlapEvents(false); // don't collide with itself
    M.MirrorSM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    M.MirrorSM->SetVisibility(true);

    M.Reflection = CreateDefaultSubobject<UPlanarReflectionComponent>(FName(*(M.Name + "Reflection")));
    M.Reflection->AttachTo(M.MirrorSM);
    M.Reflection->SetRelativeLocation(M.ReflectionPos);
    M.Reflection->SetRelativeRotation(M.ReflectionRot);
    M.Reflection->SetRelativeScale3D(M.ReflectionScale);
    M.Reflection->NormalDistortionStrength = 0.0f;
    M.Reflection->PrefilterRoughness = 0.0f;
    M.Reflection->DistanceFromPlaneFadeoutStart = 1500.f;
    M.Reflection->DistanceFromPlaneFadeoutEnd = 0.f;
    M.Reflection->AngleFromPlaneFadeStart = 90.f;
    M.Reflection->AngleFromPlaneFadeEnd = 90.f;
    M.Reflection->PrefilterRoughnessDistance = 10000.f;
    M.Reflection->ScreenPercentage = M.ScreenPercentage; // change this to reduce quality & improve performance
    M.Reflection->bShowPreviewPlane = false;
    M.Reflection->HideComponent(GetMesh());
    M.Reflection->SetVisibility(true);
}

void AEgoVehicle::InitDReyeVRMirrors()
{
    static ConstructorHelpers::FObjectFinder<UMaterial> MirrorTexture(
        TEXT("Material'/Game/Carla/Blueprints/Vehicles/DReyeVR/"
             "Mirror_DReyeVR.Mirror_DReyeVR'"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneSM(TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"));

    InitializeMirror(RearMirror, MirrorTexture.Object, PlaneSM.Object);
    InitializeMirror(LeftMirror, MirrorTexture.Object, PlaneSM.Object);
    InitializeMirror(RightMirror, MirrorTexture.Object, PlaneSM.Object);
}

FVector AEgoVehicle::GetFPSPosn() const
{
    /// TODO: refactor
    return FirstPersonCam->GetComponentLocation();
}

FRotator AEgoVehicle::GetFPSRot() const
{
    /// TODO: refactor
    return FirstPersonCam->GetComponentRotation();
}

FVector AEgoVehicle::GetCameraOffset() const
{
    return CameraLocnInVehicle;
}

const UCameraComponent *AEgoVehicle::GetCamera() const
{
    return FirstPersonCam;
}

UCameraComponent *AEgoVehicle::GetCamera()
{
    return FirstPersonCam;
}

const USceneComponent *AEgoVehicle::GetVRCameraRoot() const
{
    return VRCameraRoot;
}

USceneComponent *AEgoVehicle::GetVRCameraRoot()
{
    return VRCameraRoot;
}

void AEgoVehicle::BeginPlay()
{
    // Called when the game starts or when spawned
    Super::BeginPlay();

    // Get information about the world
    World = GetWorld();
    // const FString SetVRPixelDensity = "vr.PixelDensity " + FString::SanitizeFloat(PixelDensity);
    // World->Exec(World, *SetVRPixelDensity);
    Player = UGameplayStatics::GetPlayerController(World, 0); // main player (0) controller

    // Setup the HUD
    AHUD *Raw_HUD = Player->GetHUD();
    HUD = CastChecked<ADReyeVRHUD>(Raw_HUD);
    HUD->SetPlayer(Player);

    // Get information about the VR headset
    IsHMDConnected = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
    if (IsHMDConnected)
    {
        UE_LOG(LogTemp, Warning, TEXT("HMD detected"));
        // Now we'll begin with setting up the VR Origin logic
        UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye); // Also have Floor & Stage Level
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("NO HMD detected"));
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
    // Register Ego Vehicle with ActorRegistry
    FActorView::IdType ID = 512;
    FActorDescription EgoDescr;
    EgoDescr.Id = "vehicle.dreyevr";
    UCarlaStatics::GetCurrentEpisode(World)->RegisterActor(*this, EgoDescr, ID);

    UE_LOG(LogTemp, Log, TEXT("Successfully initialized DReyeVR player!"));
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
    if (bDrawDebugEditor)
    {
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
    }
#endif
    if (DrawGazeOnHUD && HUD != nullptr)
    {
        // Draw line components in HUD
        HUD->DrawDynamicLine(CombinedOrigin, CombinedOrigin + 10.f * WorldRot.RotateVector(CombinedGaze), FColor::Red,
                             3.0f);
    }
}

void AEgoVehicle::InitReticleTexture()
{
    // Used to initialize any bitmap-based image that will be used as a reticle
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
            if (bRectangularReticle && HUD != nullptr)
            {
                HUD->DrawDynamicSquare(CombinedGazePosn, 60, FColor(255, 0, 0, 255), 5);
            }
            else
            {
                // many problems here, for some reason the UE4 hud's DrawSimpleTexture function
                // crashes the thread its on by invalidating the ReticleTexture->Resource which is
                // non-const (but should be!!) This has to be a bug in UE4 code that we unfortunately have
                // to work around
                if (!ensure(ReticleTexture) || !ensure(ReticleTexture->Resource))
                {
                    InitReticleTexture();
                }
                if (HUD != nullptr && ReticleTexture != nullptr && ReticleTexture->Resource != nullptr)
                {
                    /// TODO: add scale
                    HUD->DrawReticle(ReticleTexture,
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
}

void AEgoVehicle::UpdateText()
{
    if (Player == nullptr)
        return;
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
