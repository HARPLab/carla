#include "LevelScript.h"
#include "Carla/Game/CarlaStatics.h"           // UCarlaStatics::GetRecorder
#include "Carla/Sensor/DReyeVRSensor.h"        // ADReyeVRSensor
#include "Carla/Vehicle/CarlaWheeledVehicle.h" // ACarlaWheeledVehicle
#include "Components/AudioComponent.h"         // UAudioComponent
#include "EgoVehicle.h"                        // AEgoVehicle
#include "HeadMountedDisplayFunctionLibrary.h" // IsHeadMountedDisplayAvailable
#include "Kismet/GameplayStatics.h"            // GetPlayerController
#include "Periph.h"                            // PeriphSystem
#include "UObject/UObjectIterator.h"           // TObjectInterator

ADReyeVRLevel::ADReyeVRLevel(FObjectInitializer const &FO) : Super(FO)
{
    // initialize stuff here
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    ReadConfigValue("Level", "EgoVolumePercent", EgoVolumePercent);
    ReadConfigValue("Level", "NonEgoVolumePercent", NonEgoVolumePercent);
    ReadConfigValue("Level", "AmbientVolumePercent", AmbientVolumePercent);
    ReadConfigValue("PeripheralTarget", "RotationOffset", PeriphRotationOffset);

    // Recorder/replayer
    ReadConfigValue("Replayer", "RunSyncReplay", bReplaySync);
}

void ADReyeVRLevel::BeginPlay()
{
    Super::ReceiveBeginPlay();

    // Initialize player
    Player = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    // Can we tick?
    SetActorTickEnabled(false); // make sure we do not tick ourselves

    // enable input tracking
    InputEnabled();

    // set all the volumes (ego, non-ego, ambient/world)
    SetVolume();

    // start input mapping
    SetupPlayerInputComponent();

    // Find the ego vehicle in the world
    /// TODO: optionally, spawn ego-vehicle here with parametrized inputs
    FindEgoVehicle();

    // Initialize DReyeVR spectator
    SetupSpectator();

    // Initialize periph stimuli system
    PS.Initialize(GetWorld());

    // Initialize control mode
    /// TODO: read in initial control mode from .ini
    ControlMode = DRIVER::HUMAN;
    switch (ControlMode)
    {
    case (DRIVER::HUMAN):
        PossessEgoVehicle();
        break;
    case (DRIVER::SPECTATOR):
        PossessSpectator();
        break;
    case (DRIVER::AI):
        HandoffDriverToAI();
        break;
    }
}

bool ADReyeVRLevel::FindEgoVehicle()
{
    if (EgoVehiclePtr != nullptr)
        return true;
    TArray<AActor *> FoundEgoVehicles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEgoVehicle::StaticClass(), FoundEgoVehicles);
    for (AActor *Vehicle : FoundEgoVehicles)
    {
        UE_LOG(LogTemp, Log, TEXT("Found EgoVehicle in world: %s"), *(Vehicle->GetName()));
        EgoVehiclePtr = CastChecked<AEgoVehicle>(Vehicle);
        EgoVehiclePtr->SetLevel(this);
        if (!AI_Player)
            AI_Player = EgoVehiclePtr->GetController();
        /// TODO: handle multiple ego-vehcles? (we should only ever have one!)
        return true;
    }
    UE_LOG(LogTemp, Error, TEXT("Did not find EgoVehicle"));
#if 0
    UE_LOG(LogTemp, Log, TEXT("Did not find EgoVehicle, spawning"));
    FActorSpawnParameters EgoVehicleSpawnInfo;
    EgoVehicleSpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    FTransform Spawn = GetSpawnPoint();
    FSoftObjectPath Ref(
        "Blueprint'/Game/Carla/Blueprints/Vehicles/DReyeVR/BP_EgoVehicle_DReyeVR.BP_EgoVehicle_DReyeVR'");
    UObject *Obj = Ref.ResolveObject();
    UBlueprint *Blueprint = Cast<UBlueprint>(Obj);
    if (Blueprint != nullptr)
    {
        EgoVehiclePtr = GetWorld()->SpawnActor<AEgoVehicle>(Blueprint->GeneratedClass, // TODO: refactor so
                                                            Spawn.GetLocation(),       // that the EgoVehicle class
                                                            Spawn.Rotator(),           // contains its own vehicle mesh
                                                            EgoVehicleSpawnInfo);      // properties without BP
    }
    if (EgoVehiclePtr == nullptr)
        UE_LOG(LogTemp, Error, TEXT("Spawning EgoVehicle failed!"))
#endif
    return (EgoVehiclePtr != nullptr);
}

void ADReyeVRLevel::SetupSpectator()
{
    /// TODO: fix bug where HMD is not detected on package BeginPlay()
    // if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
    const bool bEnableVRSpectator = true;
    if (bEnableVRSpectator)
    {
        FVector SpawnLocn;
        FRotator SpawnRotn;
        if (EgoVehiclePtr != nullptr)
        {
            SpawnLocn = EgoVehiclePtr->GetCameraPosn();
            SpawnRotn = EgoVehiclePtr->GetCameraRot();
        }
        // create new spectator pawn
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.ObjectFlags |= RF_Transient;
        SpectatorPtr = GetWorld()->SpawnActor<ASpectatorPawn>(ASpectatorPawn::StaticClass(), // spectator
                                                              SpawnLocn, SpawnRotn, SpawnParams);
    }
    else
    {
        UCarlaEpisode *Episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
        if (Episode != nullptr)
            SpectatorPtr = Episode->GetSpectatorPawn();
        else
        {
            if (Player != nullptr)
            {
                SpectatorPtr = Player->GetPawn();
            }
        }
    }
}

void ADReyeVRLevel::BeginDestroy()
{
    Super::BeginDestroy();
    UE_LOG(LogTemp, Log, TEXT("Finished Level"));
}

void ADReyeVRLevel::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    /// TODO: clean up replay init
    if (!bRecorderInitiated) // can't do this in constructor
    {
        // Initialize recorder/replayer
        SetupReplayer(); // once this is successfully run, it no longer gets executed
    }
    if (EgoVehiclePtr && SpectatorPtr && AI_Player)
    {
        if (ControlMode == DRIVER::AI) // when AI is controlling EgoVehicle
        {
            // Physically attach the Controller to the specified Pawn, so that our position reflects theirs.
            // const FVector &NewVehiclePosn = EgoVehiclePtr->GetNextCameraPosn(DeltaSeconds); // for pre-physics tick
            const FVector &NewVehiclePosn = EgoVehiclePtr->GetCameraPosn(); // for post-physics tick
            SpectatorPtr->SetActorLocationAndRotation(NewVehiclePosn, EgoVehiclePtr->GetCameraRot());
        }
        PS.Tick(DeltaSeconds, ADReyeVRSensor::bIsReplaying, EgoVehiclePtr->IsInCleanRoom(), EgoVehiclePtr->GetCamera());
    }
}

void ADReyeVRLevel::SetupPlayerInputComponent()
{
    InputComponent = NewObject<UInputComponent>(this);
    InputComponent->RegisterComponent();
    // set up gameplay key bindings
    check(InputComponent);
    // InputComponent->BindAction("ToggleCamera", IE_Pressed, this, &ADReyeVRLevel::ToggleSpectator);
    InputComponent->BindAction("PlayPause_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::PlayPause);
    InputComponent->BindAction("FastForward_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::FastForward);
    InputComponent->BindAction("Rewind_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::Rewind);
    InputComponent->BindAction("Restart_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::Restart);
    InputComponent->BindAction("Incr_Timestep_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::IncrTimestep);
    InputComponent->BindAction("Decr_Timestep_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::DecrTimestep);
    // Driver Handoff examples
    InputComponent->BindAction("EgoVehicle_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::PossessEgoVehicle);
    InputComponent->BindAction("Spectator_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::PossessSpectator);
    InputComponent->BindAction("AI_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::HandoffDriverToAI);
}

void ADReyeVRLevel::PossessEgoVehicle()
{
    if (!EgoVehiclePtr)
    {
        UE_LOG(LogTemp, Error, TEXT("No EgoVehicle to possess. Searching..."));
        if (!FindEgoVehicle())
            return;
    }
    check(EgoVehiclePtr != nullptr);
    // repossess the ego vehicle
    Player->Possess(EgoVehiclePtr);
    UE_LOG(LogTemp, Log, TEXT("Possessing DReyeVR EgoVehicle"));
    this->ControlMode = DRIVER::HUMAN;
}

void ADReyeVRLevel::PossessSpectator()
{
    // check if already possessing spectator
    if (Player->GetPawn() == SpectatorPtr && ControlMode != DRIVER::AI)
        return;
    if (!SpectatorPtr)
    {
        UE_LOG(LogTemp, Error, TEXT("No spectator to possess"));
        SetupSpectator();
        if (SpectatorPtr == nullptr)
        {
            return;
        }
    }
    if (EgoVehiclePtr)
    {
        // spawn from EgoVehicle head position
        const FVector &EgoLocn = EgoVehiclePtr->GetCameraPosn();
        const FRotator &EgoRotn = EgoVehiclePtr->GetCameraRot();
        SpectatorPtr->SetActorLocationAndRotation(EgoLocn, EgoRotn);
    }
    // repossess the ego vehicle
    Player->Possess(SpectatorPtr);
    UE_LOG(LogTemp, Log, TEXT("Possessing spectator player"));
    this->ControlMode = DRIVER::SPECTATOR;
}

void ADReyeVRLevel::HandoffDriverToAI()
{
    if (!EgoVehiclePtr)
    {
        UE_LOG(LogTemp, Error, TEXT("No EgoVehicle for AI handoff. Searching... "));
        if (!FindEgoVehicle())
            return;
    }
    if (!AI_Player)
    {
        UE_LOG(LogTemp, Error, TEXT("No EgoVehicle for AI handoff"));
        return;
    }
    check(AI_Player != nullptr);
    PossessSpectator();
    AI_Player->Possess(EgoVehiclePtr);
    UE_LOG(LogTemp, Log, TEXT("Handoff to AI driver"));
    this->ControlMode = DRIVER::AI;
}

void ADReyeVRLevel::PlayPause()
{
    UE_LOG(LogTemp, Log, TEXT("Toggle Play-Pause"));
    UCarlaStatics::GetRecorder(GetWorld())->RecPlayPause();
}

void ADReyeVRLevel::FastForward()
{
    UCarlaStatics::GetRecorder(GetWorld())->RecFastForward();
}

void ADReyeVRLevel::Rewind()
{
    if (UCarlaStatics::GetRecorder(GetWorld()))
        UCarlaStatics::GetRecorder(GetWorld())->RecRewind();
}

void ADReyeVRLevel::Restart()
{
    UE_LOG(LogTemp, Log, TEXT("Restarting recording"));
    if (UCarlaStatics::GetRecorder(GetWorld()))
        UCarlaStatics::GetRecorder(GetWorld())->RecRestart();
}

void ADReyeVRLevel::IncrTimestep()
{
    if (UCarlaStatics::GetRecorder(GetWorld()))
        UCarlaStatics::GetRecorder(GetWorld())->IncrTimeFactor(0.1);
}

void ADReyeVRLevel::DecrTimestep()
{
    if (UCarlaStatics::GetRecorder(GetWorld()))
        UCarlaStatics::GetRecorder(GetWorld())->IncrTimeFactor(-0.1);
}

void ADReyeVRLevel::SetupReplayer()
{
    if (UCarlaStatics::GetRecorder(GetWorld()) && UCarlaStatics::GetRecorder(GetWorld())->GetReplayer())
    {
        UCarlaStatics::GetRecorder(GetWorld())->GetReplayer()->SetSyncMode(bReplaySync);
        bRecorderInitiated = true;
    }
}

void ADReyeVRLevel::LegacyReplayPeriph(const DReyeVR::AggregateData &RecorderData, const double Per)
{
    // treat the periph ball target as a CustomActor
    // visibility triggers spawning/destroying the actor
    const DReyeVR::LegacyPeriphDataStruct &LegacyData = RecorderData.GetLegacyPeriphData();
    const std::string Name = "Legacy_PeriphBall";
    if (LegacyData.Visible)
    {
        DReyeVR::CustomActorData PeriphBall;
        PeriphBall.Name = FString(UTF8_TO_TCHAR(Name.c_str()));
        const FRotator PeriphRotation{LegacyData.head2target_pitch, LegacyData.head2target_yaw, 0.f};
        const FVector RotVecDirection =
            RecorderData.GetCameraRotationAbs().RotateVector((PeriphRotationOffset + PeriphRotation).Vector());
        PeriphBall.Location = LegacyData.WorldPos + RotVecDirection * 3.f * 100.f;
        PeriphBall.Scale3D = 0.05f * FVector::OneVector;
        PeriphBall.TypeId = static_cast<char>(DReyeVR::CustomActorData::Types::PERIPH_TARGET);
        this->ReplayCustomActor(PeriphBall, Per);
    }
    else
    {
        if (ADReyeVRCustomActor::ActiveCustomActors.find(Name) != ADReyeVRCustomActor::ActiveCustomActors.end())
            ADReyeVRCustomActor::ActiveCustomActors[Name]->RequestDestroy();
    }
}

void ADReyeVRLevel::ReplayCustomActor(const DReyeVR::CustomActorData &RecorderData, const double Per)
{
    // first spawn the actor if not currently active
    const std::string ActorName = TCHAR_TO_UTF8(*RecorderData.Name);
    ADReyeVRCustomActor *A = nullptr;
    if (ADReyeVRCustomActor::ActiveCustomActors.find(ActorName) == ADReyeVRCustomActor::ActiveCustomActors.end())
    {
        switch (RecorderData.TypeId)
        {
        case static_cast<char>(DReyeVR::CustomActorData::Types::SPHERE):
            A = ABall::RequestNewActor(GetWorld(), RecorderData.Name);
            break;
        case static_cast<char>(DReyeVR::CustomActorData::Types::CROSS):
            A = ACross::RequestNewActor(GetWorld(), RecorderData.Name);
            break;
        case static_cast<char>(DReyeVR::CustomActorData::Types::PERIPH_TARGET):
            A = APeriphTarget::RequestNewActor(GetWorld(), RecorderData.Name);
            break;
        /// TODO: generalize for other types (templates?? :eyes:)
        default:
            break; // ignore unknown actors
        }
    }
    else
    {
        A = ADReyeVRCustomActor::ActiveCustomActors[ActorName];
    }
    // ensure the actor is currently active (spawned)
    // now that we know this actor exists, update its internals
    if (A != nullptr)
        A->SetInternals(RecorderData);
}

void ADReyeVRLevel::SetVolume()
{
    // update the non-ego volume percent
    ACarlaWheeledVehicle::Volume = NonEgoVolumePercent / 100.f;

    // for all in-world audio components such as ambient birdsong, fountain splashing, smoke, etc.
    for (TObjectIterator<UAudioComponent> Itr; Itr; ++Itr)
    {
        if (Itr->GetWorld() != GetWorld()) // World Check
        {
            continue;
        }
        Itr->SetVolumeMultiplier(AmbientVolumePercent / 100.f);
    }

    // for all in-world vehicles (including the EgoVehicle) manually set their volumes
    TArray<AActor *> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACarlaWheeledVehicle::StaticClass(), FoundActors);
    for (AActor *A : FoundActors)
    {
        ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(A);
        if (Vehicle != nullptr)
        {
            float NewVolume = ACarlaWheeledVehicle::Volume; // Non ego volume
            if (Vehicle->IsA(AEgoVehicle::StaticClass()))   // dynamic cast, requires -frrti
                NewVolume = EgoVolumePercent / 100.f;
            Vehicle->SetVolume(NewVolume);
        }
    }
}

FTransform ADReyeVRLevel::GetSpawnPoint(int SpawnPointIndex) const
{
    ACarlaGameModeBase *GM = UCarlaStatics::GetGameMode(GetWorld());
    if (GM != nullptr)
    {
        TArray<FTransform> SpawnPoints = GM->GetSpawnPointsTransforms();
        size_t WhichPoint = 0; // default to first one
        if (SpawnPointIndex < 0)
            WhichPoint = FMath::RandRange(0, SpawnPoints.Num());
        else
            WhichPoint = FMath::Clamp(SpawnPointIndex, 0, SpawnPoints.Num());

        if (WhichPoint < SpawnPoints.Num()) // SpawnPoints could be empty
            return SpawnPoints[WhichPoint];
    }
    /// TODO: return a safe bet (position of the player start maybe?)
    return FTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector::OneVector);
}