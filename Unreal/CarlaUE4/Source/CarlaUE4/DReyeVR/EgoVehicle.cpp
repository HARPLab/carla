#include "EgoVehicle.h"
#include "Carla/Actor/ActorAttribute.h"             // FActorAttribute
#include "Carla/Actor/ActorRegistry.h"              // Register
#include "Carla/Game/CarlaStatics.h"                // GetEpisode
#include "Carla/Vehicle/CarlaWheeledVehicleState.h" // ECarlaWheeledVehicleState
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
    PrimaryActorTick.TickGroup = TG_PostPhysics;

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
}

void AEgoVehicle::ReadConfigVariables()
{
    ReadConfigValue("EgoVehicle", "CameraInit", CameraLocnInVehicle);
    ReadConfigValue("EgoVehicle", "DashLocation", DashboardLocnInVehicle);
    // vr
    ReadConfigValue("EgoVehicle", "FieldOfView", FieldOfView);
    // inputs
    ReadConfigValue("Vehicle", "ScaleSteeringDamping", ScaleSteeringInput);
    ReadConfigValue("Vehicle", "ScaleThrottleInput", ScaleThrottleInput);
    ReadConfigValue("Vehicle", "ScaleBrakeInput", ScaleBrakeInput);
    ReadConfigValue("Vehicle", "InvertMouseY", InvertMouseY);
    ReadConfigValue("Vehicle", "ScaleMouseY", ScaleMouseY);
    ReadConfigValue("Vehicle", "ScaleMouseX", ScaleMouseX);
    // HUD (Head's Up Display)
    ReadConfigValue("VehicleHUD", "DrawFPSCounter", bDrawFPSCounter);
    ReadConfigValue("VehicleHUD", "DrawFlatReticle", bDrawFlatReticle);
    ReadConfigValue("VehicleHUD", "ReticleSize", ReticleSize);
    ReadConfigValue("VehicleHUD", "DrawGaze", bDrawGaze);
    ReadConfigValue("VehicleHUD", "DrawSpectatorReticle", bDrawSpectatorReticle);
    ReadConfigValue("VehicleHUD", "EnableSpectatorScreen", bEnableSpectatorScreen);

    // cosmetic
    ReadConfigValue("EgoVehicle", "DrawDebugEditor", bDrawDebugEditor);
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
    UBoxComponent *Bounds = this->GetVehicleBoundingBox();
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
    EngineRevSound->bAutoActivate = true;          // start playing on begin
    EngineRevSound->SetSound(EgoEngineCue.Object); // using this sound

    // Initialize audio components
    static ConstructorHelpers::FObjectFinder<USoundWave> GearSound(
        TEXT("SoundWave'/Game/Carla/Blueprints/Vehicles/DReyeVR/Sounds/GearShift.GearShift'"));
    GearShiftSound = CreateDefaultSubobject<UAudioComponent>(TEXT("GearShift"));
    GearShiftSound->SetupAttachment(GetRootComponent());
    GearShiftSound->bAutoActivate = false;
    GearShiftSound->SetSound(GearSound.Object);

    static ConstructorHelpers::FObjectFinder<USoundWave> TurnSignalSoundWave(
        TEXT("SoundWave'/Game/Carla/Blueprints/Vehicles/DReyeVR/Sounds/TurnSignal.TurnSignal'"));
    TurnSignalSound = CreateDefaultSubobject<UAudioComponent>(TEXT("TurnSignal"));
    TurnSignalSound->SetupAttachment(GetRootComponent());
    TurnSignalSound->bAutoActivate = false;
    TurnSignalSound->SetSound(TurnSignalSoundWave.Object);

    static ConstructorHelpers::FObjectFinder<USoundWave> CarCrashSound(
        TEXT("SoundWave'/Game/Carla/Blueprints/Vehicles/DReyeVR/Sounds/Crash.Crash'"));
    CrashSound = CreateDefaultSubobject<UAudioComponent>(TEXT("CarCrash"));
    CrashSound->SetupAttachment(GetRootComponent());
    CrashSound->bAutoActivate = false;
    CrashSound->SetSound(CarCrashSound.Object);
}

void AEgoVehicle::PlayGearShiftSound(const float DelayBeforePlay) const
{
    if (this->GearShiftSound)
        GearShiftSound->Play(DelayBeforePlay);
}

void AEgoVehicle::PlayTurnSignalSound(const float DelayBeforePlay) const
{
    if (this->TurnSignalSound)
        this->TurnSignalSound->Play(DelayBeforePlay);
}

void AEgoVehicle::PlayCrashSound(const float DelayBeforePlay) const
{
    if (this->CrashSound)
        this->CrashSound->Play(DelayBeforePlay);
}

void AEgoVehicle::SetVolume(const float VolumeIn)
{
    if (GearShiftSound)
        GearShiftSound->SetVolumeMultiplier(VolumeIn);
    if (TurnSignalSound)
        TurnSignalSound->SetVolumeMultiplier(VolumeIn);
    if (CrashSound)
        CrashSound->SetVolumeMultiplier(VolumeIn);
    Super::SetVolume(VolumeIn);
}

FVector AEgoVehicle::GetCameraPosn() const
{
    return FirstPersonCam->GetComponentLocation();
}

FVector AEgoVehicle::GetNextCameraPosn(const float DeltaSeconds) const
{
    // usually only need this is tick before physics
    return this->GetCameraPosn() + DeltaSeconds * this->GetVelocity();
}

FRotator AEgoVehicle::GetCameraRot() const
{
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
    Player = UGameplayStatics::GetPlayerController(World, 0); // main player (0) controller
    Episode = UCarlaStatics::GetCurrentEpisode(World);

    // Setup the HUD
    AHUD *Raw_HUD = Player->GetHUD();
    HUD = Cast<ADReyeVRHUD>(Raw_HUD);
    if (HUD)
    {
        HUD->SetPlayer(Player);
    }

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
    EgoSensor = World->SpawnActor<AEgoSensor>(FirstPersonCam->GetComponentLocation(), FRotator(0.0f, 0.0f, 0.0f),
                                              EyeTrackerSpawnInfo);
    // Attach the EgoSensor as a child to the EgoVehicle BP
    EgoSensor->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
    EgoSensor->SetPlayer(Player);
    EgoSensor->SetCamera(FirstPersonCam);

    // Enable VR spectator screen & eye reticle
    if (!bEnableSpectatorScreen)
    {
        UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenMode(ESpectatorScreenMode::Disabled);
    }
    else if (bDrawSpectatorReticle && IsHMDConnected)
    {
        InitReticleTexture(); // generate array of pixel values
        check(ReticleTexture);
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
    this->GetVehicleMovementComponent()->SetTargetGear(1, true);

    // Register Ego Vehicle with ActorRegistry
    Register();

    UE_LOG(LogTemp, Log, TEXT("Initialized DReyeVR EgoVehicle"));
}

void AEgoVehicle::Register()
{
    /// TODO: parametrize
    FActorView::IdType ID = EgoVehicleID;
    FActorDescription Description;
    Description.Class = ACarlaWheeledVehicle::StaticClass();
    Description.Id = "vehicle.dreyevr.egovehicle";
    Description.UId = ID;
    // ensure this vehicle is denoted by the 'hero' attribute
    FActorAttribute HeroRole;
    HeroRole.Id = "role_name";
    HeroRole.Type = EActorAttributeType::String;
    HeroRole.Value = "hero";
    Description.Variations.Add(HeroRole.Id, std::move(HeroRole));
    // ensure the vehicle has attributes denoting number of wheels
    FActorAttribute NumWheels;
    NumWheels.Id = "number_of_wheels";
    NumWheels.Type = EActorAttributeType::Int;
    NumWheels.Value = "4";
    Description.Variations.Add(NumWheels.Id, std::move(NumWheels));
    FString Tags = "EgoVehicle,DReyeVR";
    Episode->RegisterActor(*this, Description, Tags, ID);
}

void AEgoVehicle::BeginDestroy()
{
    // destroy all spawned entities
    if (EgoSensor)
        EgoSensor->Destroy();

    Super::BeginDestroy();
}

void AEgoVehicle::ReplayUpdate()
{
    // perform all updates that occur when replaying
    if (EgoSensor->IsReplaying())
    {
        // if we don't have this positional update here, there is lag/jitter between the FPS camera and the vehicle
        // since they are likely updating on different ticks (Carla Replayers's vs here)
        const FTransform ReplayTransform(EgoSensor->GetData()->GetVehicleRotation(), // FRotator (Rotation)
                                         EgoSensor->GetData()->GetVehicleLocation(), // FVector (Location)
                                         FVector::OneVector);                        // FVector (Scale3D)
        SetActorTransform(ReplayTransform, false, nullptr, ETeleportType::None);
    }
}

// Called every frame
void AEgoVehicle::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Update the positions based off replay data
    ReplayUpdate();

    // Update sensor
    UpdateSensor(DeltaSeconds);

    // Draw debug lines on editor
    DebugLines();

    // Render EgoVehicle dashboard
    UpdateDash();

    // Draw the flat-screen HUD items like eye-reticle and FPS counter
    DrawFlatHUD(DeltaSeconds); // NOTE: only present in flat-screen mode!

    // Draw the spectator vr screen and overlay elements
    DrawSpectatorScreen();

    // Update the world level
    TickLevel(DeltaSeconds);

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
    if (!EgoSensor->IsReplaying())
    {
        EgoSensor->SetInputs(VehicleInputs);
        EgoSensor->SetEgoVelocity(GetVehicleForwardSpeed());
        EgoSensor->SetEgoTransform(GetActorTransform());
    }
    else
    {
        // this gets reached when the simulator is replaying data from a carla log
        const DReyeVR::AggregateData *Replay = EgoSensor->GetData();
        // assign first person camera orientation and location
        FirstPersonCam->SetRelativeRotation(Replay->GetCameraRotation(), false, nullptr, ETeleportType::None);
        FirstPersonCam->SetRelativeLocation(Replay->GetCameraLocation(), false, nullptr, ETeleportType::None);
    }
    VehicleInputs = {}; // clear inputs to be updated on the next tick

    // Calculate gaze data using eye tracker data
    const DReyeVR::AggregateData *Data = EgoSensor->GetData();
    // Compute World positions and orientations
    const FRotator WorldRot = FirstPersonCam->GetComponentRotation();
    const FVector WorldPos = FirstPersonCam->GetComponentLocation();

    // First get the gaze origin and direction and vergence from the EyeTracker Sensor
    const float Vergence = FMath::Max(1.f, Data->GetGazeVergence() / 100.f); // vergence to m (from cm)
    const float UE4MeterScale = UHeadMountedDisplayFunctionLibrary::GetWorldToMetersScale(World);

    // Both eyes
    CombinedGaze = Vergence * UE4MeterScale * Data->GetGazeDir();
    CombinedOrigin = WorldRot.RotateVector(Data->GetGazeOrigin()) + WorldPos;

    // Left eye
    LeftGaze = UE4MeterScale * Data->GetGazeDir(DReyeVR::Gaze::LEFT);
    LeftOrigin = WorldRot.RotateVector(Data->GetGazeOrigin(DReyeVR::Gaze::LEFT)) + WorldPos;

    // Right eye
    RightGaze = UE4MeterScale * Data->GetGazeDir(DReyeVR::Gaze::RIGHT);
    RightOrigin = WorldRot.RotateVector(Data->GetGazeOrigin(DReyeVR::Gaze::RIGHT)) + WorldPos;
}

/// ========================================== ///
/// -----------------:LEVEL:------------------ ///
/// ========================================== ///

void AEgoVehicle::SetLevel(ADReyeVRLevel *Level)
{
    this->DReyeVRLevel = Level;
}

void AEgoVehicle::TickLevel(float DeltaSeconds)
{
    if (this->DReyeVRLevel == nullptr)
        return;
    DReyeVRLevel->Tick(DeltaSeconds);
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
    if (bDrawGaze && HUD != nullptr)
    {
        // Draw line components in HUD
        HUD->DrawDynamicLine(CombinedOrigin, CombinedOrigin + 10.f * WorldRot.RotateVector(CombinedGaze), FColor::Red,
                             3.0f);
    }
}

void AEgoVehicle::InitReticleTexture()
{
    // Used to initialize any bitmap-based image that will be used as a reticle
    ReticleSrc.Reserve(ReticleSize * ReticleSize); // allocate width*height space
    for (int i = 0; i < ReticleSize; i++)
    {
        for (int j = 0; j < ReticleSize; j++)
        {
            // RGBA colours
            FColor Colour;
            if (bRectangularReticle)
            {
                const int ReticleThickness = ReticleSize / 10;
                bool LeftOrRight = (i < ReticleThickness || i > ReticleSize - ReticleThickness);
                bool TopOrBottom = (j < ReticleThickness || j > ReticleSize - ReticleThickness);
                if (LeftOrRight || TopOrBottom)
                    Colour = FColor(255, 0, 0, 128); // (semi-opaque red)
                else
                    Colour = FColor(0, 0, 0, 0); // (fully transparent inside)
            }
            else
            {
                const int x = i - ReticleSize / 2;
                const int y = j - ReticleSize / 2;
                const float Radius = ReticleSize / 3.f;
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
    ReticleTexture = UTexture2D::CreateTransient(ReticleSize, ReticleSize, PF_B8G8R8A8);
    void *TextureData = ReticleTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(TextureData, ReticleSrc.GetData(), 4 * ReticleSize * ReticleSize);
    ReticleTexture->PlatformData->Mips[0].BulkData.Unlock();
    ReticleTexture->UpdateResource();
    // ReticleTexture = FImageUtils::CreateTexture2D(ReticleSize, ReticleSize, ReticleSrc, GetWorld(),
    //                                               "EyeReticleTexture", EObjectFlags::RF_Transient, params);

    check(ReticleTexture);
    check(ReticleTexture->Resource);
}

void AEgoVehicle::DrawFlatHUD(float DeltaSeconds)
{
    if (HUD == nullptr || Player == nullptr)
        return;
    // calculate View size (of open window). Note this is not the same as resolution
    FIntPoint ViewSize;
    Player->GetViewportSize(ViewSize.X, ViewSize.Y);
    // Get eye tracker variables
    const FRotator WorldRot = GetCamera()->GetComponentRotation();
    const FVector CombinedGazePosn = CombinedOrigin + WorldRot.RotateVector(this->CombinedGaze);
    // Draw elements of the HUD
    if (bDrawFlatReticle) // Draw reticle on flat-screen HUD
    {
        const float Diameter = ReticleSize;
        const float Thickness = (ReticleSize / 2.f) / 10.f; // 10 % of radius
        if (bRectangularReticle)
        {
            HUD->DrawDynamicSquare(CombinedGazePosn, Diameter, FColor(255, 0, 0, 255), Thickness);
        }
        else
        {
            HUD->DrawDynamicCrosshair(CombinedGazePosn, Diameter, FColor(255, 0, 0, 255), true, Thickness);
#if 0
            // many problems here, for some reason the UE4 hud's DrawSimpleTexture function
            // crashes the thread its on by invalidating the ReticleTexture->Resource which is
            // non-const (but should be!!) This has to be a bug in UE4 code that we unfortunately have
            // to work around
            if (!ensure(ReticleTexture) || !ensure(ReticleTexture->Resource))
            {
                InitReticleTexture();
            }
            if (ReticleTexture != nullptr && ReticleTexture->Resource != nullptr)
            {
                HUD->DrawReticle(ReticleTexture, ReticlePos + FVector2D(-ReticleSize * 0.5f, -ReticleSize * 0.5f));
            }
#endif
        }
    }
    if (bDrawFPSCounter)
    {
        HUD->DrawDynamicText(FString::FromInt(int(1.f / DeltaSeconds)), FVector2D(ViewSize.X - 100, 50),
                             FColor(0, 255, 0, 213), 2);
    }
}

void AEgoVehicle::DrawSpectatorScreen()
{
    if (!bEnableSpectatorScreen || Player == nullptr || !IsHMDConnected)
        return;
    // calculate View size (of open window). Note this is not the same as resolution
    FIntPoint ViewSize;
    Player->GetViewportSize(ViewSize.X, ViewSize.Y);
    // Get eye tracker variables
    const FRotator WorldRot = GetCamera()->GetComponentRotation();
    const FVector CombinedGazePosn = CombinedOrigin + WorldRot.RotateVector(this->CombinedGaze);

    /// TODO: draw other things on the spectator screen?
    if (bDrawSpectatorReticle)
    {
        /// NOTE: this is the better way to get the ViewportSize
        FVector2D ReticlePos;
        UGameplayStatics::ProjectWorldToScreen(Player, CombinedGazePosn, ReticlePos, true);
        /// NOTE: the SetSpectatorScreenModeTexturePlusEyeLayout expects normalized positions on the screen
        /// NOTE: to get the best drawing, the texture is offset slightly by this vector
        const FVector2D ScreenOffset(ReticleSize * 0.5f, -ReticleSize);
        ReticlePos += ScreenOffset; // move X right by Dim.X/2, move Y up by Dim.Y
        // define min and max bounds
        FVector2D TextureRectMin(FMath::Clamp(ReticlePos.X / ViewSize.X, 0.f, 1.f),
                                 FMath::Clamp(ReticlePos.Y / ViewSize.Y, 0.f, 1.f));
        // max needs to define the bottom right corner, so needs to be +Dim.X right, and +Dim.Y down
        FVector2D TextureRectMax(FMath::Clamp((ReticlePos.X + ReticleSize) / ViewSize.X, 0.f, 1.f),
                                 FMath::Clamp((ReticlePos.Y + ReticleSize) / ViewSize.Y, 0.f, 1.f));
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

void AEgoVehicle::UpdateDash()
{
    if (Player == nullptr)
        return;
    // Draw text components
    float MPH;
    if (EgoSensor->IsReplaying())
    {
        const DReyeVR::AggregateData *Replay = EgoSensor->GetData();
        MPH = Replay->GetVehicleVelocity() * 0.0223694f; // cm/s to mph
        if (Replay->GetUserInputs().ToggledReverse)
        {
            bReverse = !bReverse;
            PlayGearShiftSound();
        }
        if (Replay->GetUserInputs().TurnSignalLeft)
        {
            LeftSignalTimeToDie = FPlatformTime::Seconds() + TurnSignalDuration;
            PlayTurnSignalSound();
        }
        if (Replay->GetUserInputs().TurnSignalRight)
        {
            RightSignalTimeToDie = FPlatformTime::Seconds() + TurnSignalDuration;
            PlayTurnSignalSound();
        }
    }
    else
    {
        MPH = GetVehicleForwardSpeed() * 0.0223694f; // FwdSpeed is in cm/s, mult by 0.0223694 to get mph
    }

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
