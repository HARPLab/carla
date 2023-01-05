#include "DReyeVRGameMode.h"
#include "Carla/AI/AIControllerFactory.h"      // AAIControllerFactory
#include "Carla/Actor/StaticMeshFactory.h"     // AStaticMeshFactory
#include "Carla/Game/CarlaStatics.h"           // GetRecorder, GetEpisode
#include "Carla/Sensor/DReyeVRSensor.h"        // ADReyeVRSensor
#include "Carla/Sensor/SensorFactory.h"        // ASensorFactory
#include "Carla/Trigger/TriggerFactory.h"      // TriggerFactory
#include "Carla/Vehicle/CarlaWheeledVehicle.h" // ACarlaWheeledVehicle
#include "Carla/Weather/Weather.h"             // AWeather
#include "Components/AudioComponent.h"         // UAudioComponent
#include "DReyeVRPawn.h"                       // ADReyeVRPawn
#include "EgoVehicle.h"                        // AEgoVehicle
#include "FlatHUD.h"                           // ADReyeVRHUD
#include "HeadMountedDisplayFunctionLibrary.h" // IsHeadMountedDisplayAvailable
#include "Kismet/GameplayStatics.h"            // GetPlayerController
#include "UObject/UObjectIterator.h"           // TObjectInterator

ADReyeVRGameMode::ADReyeVRGameMode(FObjectInitializer const &FO) : Super(FO)
{
    // initialize stuff here
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    // initialize default classes
    this->HUDClass = ADReyeVRHUD::StaticClass();
    static ConstructorHelpers::FObjectFinder<UBlueprint> WeatherBP(
        TEXT("Blueprint'/Game/Carla/Blueprints/Weather/BP_Weather.BP_Weather'"));
    this->WeatherClass = WeatherBP.Object->GeneratedClass;

    // initialize actor factories
    // https://forums.unrealengine.com/t/what-is-the-right-syntax-of-fclassfinder-and-how-could-i-generaly-use-it-to-find-a-specific-blueprint/363884
    static ConstructorHelpers::FClassFinder<ACarlaActorFactory> VehicleFactoryBP(
        TEXT("Blueprint'/Game/Carla/Blueprints/Vehicles/VehicleFactory'"));
    static ConstructorHelpers::FClassFinder<ACarlaActorFactory> WalkerFactoryBP(
        TEXT("Blueprint'/Game/Carla/Blueprints/Walkers/WalkerFactory'"));
    static ConstructorHelpers::FClassFinder<ACarlaActorFactory> PropFactoryBP(
        TEXT("Blueprint'/Game/Carla/Blueprints/Props/PropFactory'"));

    this->ActorFactories = TSet<TSubclassOf<ACarlaActorFactory>>{
        VehicleFactoryBP.Class,
        ASensorFactory::StaticClass(),
        WalkerFactoryBP.Class,
        PropFactoryBP.Class,
        ATriggerFactory::StaticClass(),
        AAIControllerFactory::StaticClass(),
        AStaticMeshFactory::StaticClass(),
    };

    // read config variables
    ReadConfigValue("Level", "EgoVolumePercent", EgoVolumePercent);
    ReadConfigValue("Level", "NonEgoVolumePercent", NonEgoVolumePercent);
    ReadConfigValue("Level", "AmbientVolumePercent", AmbientVolumePercent);

    // Recorder/replayer
    ReadConfigValue("Replayer", "RunSyncReplay", bReplaySync);

    // get ego vehicle bp
    static ConstructorHelpers::FObjectFinder<UBlueprint> EgoVehicleBP(
        TEXT("Blueprint'/Game/Carla/Blueprints/Vehicles/DReyeVR/BP_EgoVehicle_DReyeVR.BP_EgoVehicle_DReyeVR'"));
    EgoVehicleBPClass = static_cast<UClass *>(EgoVehicleBP.Object->GeneratedClass);
}

void ADReyeVRGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Initialize player
    Player = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    // Can we tick?
    SetActorTickEnabled(false); // make sure we do not tick ourselves

    // set all the volumes (ego, non-ego, ambient/world)
    SetVolume();

    // start input mapping
    SetupPlayerInputComponent();

    // spawn the DReyeVR pawn and possess it
    StartDReyeVRPawn();

    // Find the ego vehicle in the world
    /// TODO: optionally, spawn ego-vehicle here with parametrized inputs
    SetupEgoVehicle();

    // Initialize DReyeVR spectator
    SetupSpectator();

    // Initialize control mode
    ControlMode = DRIVER::HUMAN;
}

void ADReyeVRGameMode::StartDReyeVRPawn()
{
    FActorSpawnParameters S;
    DReyeVR_Pawn = GetWorld()->SpawnActor<ADReyeVRPawn>(FVector::ZeroVector, FRotator::ZeroRotator, S);
    /// NOTE: the pawn is automatically possessed by player0
    // as the constructor has the AutoPossessPlayer != disabled
    // if you want to manually possess then you can do Player->Possess(DReyeVR_Pawn);
    ensure(DReyeVR_Pawn != nullptr);
}

bool ADReyeVRGameMode::SetupEgoVehicle()
{
    if (EgoVehiclePtr != nullptr)
        return true;
    ensure(DReyeVR_Pawn);

    TArray<AActor *> FoundEgoVehicles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEgoVehicle::StaticClass(), FoundEgoVehicles);
    if (FoundEgoVehicles.Num() > 0)
    {
        for (AActor *Vehicle : FoundEgoVehicles)
        {
            LOG("Found EgoVehicle in world: %s", *(Vehicle->GetName()));
            EgoVehiclePtr = CastChecked<AEgoVehicle>(Vehicle);
            /// TODO: handle multiple ego-vehcles? (we should only ever have one!)
            break;
        }
    }
    else
    {
        LOG("Did not find EgoVehicle in map... spawning...");
        auto World = GetWorld();
        check(World != nullptr);
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        FTransform SpawnPt = GetSpawnPoint();
        ensure(EgoVehicleBPClass != nullptr);
        EgoVehiclePtr =
            World->SpawnActor<AEgoVehicle>(EgoVehicleBPClass, SpawnPt.GetLocation(), SpawnPt.Rotator(), SpawnParams);
    }
    check(EgoVehiclePtr != nullptr);
    EgoVehiclePtr->SetLevel(this);
    if (DReyeVR_Pawn)
    {
        // need to assign ego vehicle before possess!
        DReyeVR_Pawn->BeginEgoVehicle(EgoVehiclePtr, GetWorld(), Player);
        LOG("Created DReyeVR controller pawn");
    }
    return (EgoVehiclePtr != nullptr);
}

void ADReyeVRGameMode::SetupSpectator()
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

void ADReyeVRGameMode::BeginDestroy()
{
    Super::BeginDestroy();
    LOG("Finished Level");
}

void ADReyeVRGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    /// TODO: clean up replay init
    if (!bRecorderInitiated) // can't do this in constructor
    {
        // Initialize recorder/replayer
        SetupReplayer(); // once this is successfully run, it no longer gets executed
    }

    DrawBBoxes();
}

void ADReyeVRGameMode::SetupPlayerInputComponent()
{
    InputComponent = NewObject<UInputComponent>(this);
    InputComponent->RegisterComponent();
    // set up gameplay key bindings
    check(InputComponent);
    // InputComponent->BindAction("ToggleCamera", IE_Pressed, this, &ADReyeVRGameMode::ToggleSpectator);
    InputComponent->BindAction("PlayPause_DReyeVR", IE_Pressed, this, &ADReyeVRGameMode::PlayPause);
    InputComponent->BindAction("FastForward_DReyeVR", IE_Pressed, this, &ADReyeVRGameMode::FastForward);
    InputComponent->BindAction("Rewind_DReyeVR", IE_Pressed, this, &ADReyeVRGameMode::Rewind);
    InputComponent->BindAction("Restart_DReyeVR", IE_Pressed, this, &ADReyeVRGameMode::Restart);
    InputComponent->BindAction("Incr_Timestep_DReyeVR", IE_Pressed, this, &ADReyeVRGameMode::IncrTimestep);
    InputComponent->BindAction("Decr_Timestep_DReyeVR", IE_Pressed, this, &ADReyeVRGameMode::DecrTimestep);
    // Driver Handoff examples
    InputComponent->BindAction("EgoVehicle_DReyeVR", IE_Pressed, this, &ADReyeVRGameMode::PossessEgoVehicle);
    InputComponent->BindAction("Spectator_DReyeVR", IE_Pressed, this, &ADReyeVRGameMode::PossessSpectator);
    InputComponent->BindAction("AI_DReyeVR", IE_Pressed, this, &ADReyeVRGameMode::HandoffDriverToAI);
}

void ADReyeVRGameMode::PossessEgoVehicle()
{
    if (Player->GetPawn() != DReyeVR_Pawn)
    {
        LOG("Possessing DReyeVR EgoVehicle");
        Player->Possess(DReyeVR_Pawn);
    }
    ensure(EgoVehiclePtr != nullptr);
    if (EgoVehiclePtr)
    {
        EgoVehiclePtr->SetAutopilot(false);
        LOG("Disabling EgoVehicle Autopilot");
        this->ControlMode = DRIVER::AI;
    }
    this->ControlMode = DRIVER::HUMAN;
}

void ADReyeVRGameMode::PossessSpectator()
{
    // check if already possessing spectator
    if (Player->GetPawn() == SpectatorPtr && ControlMode != DRIVER::AI)
        return;
    if (!SpectatorPtr)
    {
        UE_LOG(LogDReyeVR, Error, TEXT("No spectator to possess"));
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
    LOG("Possessing spectator player");
    this->ControlMode = DRIVER::SPECTATOR;
}

void ADReyeVRGameMode::HandoffDriverToAI()
{
    ensure(EgoVehiclePtr != nullptr);
    if (EgoVehiclePtr)
    {
        EgoVehiclePtr->SetAutopilot(true);
        LOG("Enabling EgoVehicle Autopilot");
        this->ControlMode = DRIVER::AI;
    }
}

void ADReyeVRGameMode::PlayPause()
{
    LOG("Toggle Play-Pause");
    UCarlaStatics::GetRecorder(GetWorld())->RecPlayPause();
}

void ADReyeVRGameMode::FastForward()
{
    UCarlaStatics::GetRecorder(GetWorld())->RecFastForward();
}

void ADReyeVRGameMode::Rewind()
{
    if (UCarlaStatics::GetRecorder(GetWorld()))
        UCarlaStatics::GetRecorder(GetWorld())->RecRewind();
}

void ADReyeVRGameMode::Restart()
{
    LOG("Restarting recording");
    if (UCarlaStatics::GetRecorder(GetWorld()))
        UCarlaStatics::GetRecorder(GetWorld())->RecRestart();
}

void ADReyeVRGameMode::IncrTimestep()
{
    if (UCarlaStatics::GetRecorder(GetWorld()))
        UCarlaStatics::GetRecorder(GetWorld())->IncrTimeFactor(0.1);
}

void ADReyeVRGameMode::DecrTimestep()
{
    if (UCarlaStatics::GetRecorder(GetWorld()))
        UCarlaStatics::GetRecorder(GetWorld())->IncrTimeFactor(-0.1);
}

void ADReyeVRGameMode::SetupReplayer()
{
    if (UCarlaStatics::GetRecorder(GetWorld()) && UCarlaStatics::GetRecorder(GetWorld())->GetReplayer())
    {
        UCarlaStatics::GetRecorder(GetWorld())->GetReplayer()->SetSyncMode(bReplaySync);
        bRecorderInitiated = true;
    }
}

void ADReyeVRGameMode::DrawBBoxes()
{
#if 0
    TArray<AActor *> FoundActors;
    if (GetWorld() != nullptr)
    {
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACarlaWheeledVehicle::StaticClass(), FoundActors);
    }
    for (AActor *A : FoundActors)
    {
        std::string name = TCHAR_TO_UTF8(*A->GetName());
        if (A->GetName().Contains("DReyeVR"))
            continue; // skip drawing a bbox over the EgoVehicle
        if (BBoxes.find(name) == BBoxes.end())
        {
            BBoxes[name] = ADReyeVRCustomActor::CreateNew(SM_CUBE, MAT_TRANSLUCENT, GetWorld(), "BBox" + A->GetName());
        }
        const float DistThresh = 20.f; // meters before nearby bounding boxes become red
        ADReyeVRCustomActor *BBox = BBoxes[name];
        ensure(BBox != nullptr);
        if (BBox != nullptr)
        {
            BBox->Activate();
            BBox->MaterialParams.Opacity = 0.1f;
            FLinearColor Col = FLinearColor::Green;
            if (FVector::Distance(EgoVehiclePtr->GetActorLocation(), A->GetActorLocation()) < DistThresh * 100.f)
            {
                Col = FLinearColor::Red;
            }
            BBox->MaterialParams.BaseColor = Col;
            BBox->MaterialParams.Emissive = 0.1 * Col;

            FVector Origin;
            FVector BoxExtent;
            A->GetActorBounds(true, Origin, BoxExtent, false);
            // LOG("Origin: %s Extent %s"), *Origin.ToString(), *BoxExtent.ToString());
            // divide by 100 to get from m to cm, multiply by 2 bc the cube is scaled in both X and Y
            BBox->SetActorScale3D(2 * BoxExtent / 100.f);
            BBox->SetActorLocation(Origin);
            // extent already covers the rotation aspect since the bbox is dynamic and axis aligned
            // BBox->SetActorRotation(A->GetActorRotation());
        }
    }
#endif
}

void ADReyeVRGameMode::ReplayCustomActor(const DReyeVR::CustomActorData &RecorderData, const double Per)
{
    // first spawn the actor if not currently active
    const std::string ActorName = TCHAR_TO_UTF8(*RecorderData.Name);
    ADReyeVRCustomActor *A = nullptr;
    if (ADReyeVRCustomActor::ActiveCustomActors.find(ActorName) == ADReyeVRCustomActor::ActiveCustomActors.end())
    {
        /// TODO: also track KnownNumMaterials?
        A = ADReyeVRCustomActor::CreateNew(RecorderData.MeshPath, RecorderData.MaterialParams.MaterialPath, GetWorld(),
                                           RecorderData.Name);
    }
    else
    {
        A = ADReyeVRCustomActor::ActiveCustomActors[ActorName];
    }
    // ensure the actor is currently active (spawned)
    // now that we know this actor exists, update its internals
    if (A != nullptr)
    {
        A->SetInternals(RecorderData);
        A->Activate();
        A->Tick(Per); // update locations immediately
    }
}

void ADReyeVRGameMode::SetVolume()
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

FTransform ADReyeVRGameMode::GetSpawnPoint(int SpawnPointIndex) const
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