#include "DReyeVRLevel.h"
#include "Carla/Game/CarlaStatics.h"           // UCarlaStatics::GetRecorder
#include "Carla/Sensor/DReyeVRSensor.h"        // ADReyeVRSensor
#include "Carla/Vehicle/CarlaWheeledVehicle.h" // ACarlaWheeledVehicle
#include "EgoVehicle.h"                        // AEgoVehicle
#include "Kismet/GameplayStatics.h"            // GetPlayerController

ADReyeVRLevel::ADReyeVRLevel()
{
    // initialize stuff here
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
}

ADReyeVRLevel::ADReyeVRLevel(FObjectInitializer const &FO) : ADReyeVRLevel()
{
    // initialize stuff here
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
}

void ADReyeVRLevel::BeginPlay()
{
    Super::ReceiveBeginPlay();

    // Initialize player
    Player = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    // Can we tick?
    SetActorTickEnabled(true); // make sure we DO tick

    // enable input tracking
    InputEnabled();

    // start input mapping
    SetupPlayerInputComponent();

    // Find the ego vehicle in the world
    TArray<AActor *> FoundEgoVehicles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEgoVehicle::StaticClass(), FoundEgoVehicles);
    if (FoundEgoVehicles.Num() > 0)
    {
        EgoVehiclePtr = Cast<AEgoVehicle>(FoundEgoVehicles[0]);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DID NOT FIND EGO VEHICLE IN WORLD!"));
    }
    check(EgoVehiclePtr != nullptr);

    // Finished with setup
    UE_LOG(LogTemp, Log, TEXT("DReyeVR Begin Play"));
}

void ADReyeVRLevel::BeginDestroy()
{
    Super::BeginDestroy();

    UE_LOG(LogTemp, Log, TEXT("Finished Level BP"));
}

void ADReyeVRLevel::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void ADReyeVRLevel::SetupPlayerInputComponent()
{
    InputComponent = NewObject<UInputComponent>(this);
    InputComponent->RegisterComponent();
    // set up gameplay key bindings
    check(InputComponent);
    InputComponent->BindAction("ToggleCamera", IE_Pressed, this, &ADReyeVRLevel::ToggleSpectator);
    InputComponent->BindAction("PlayPause_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::PlayPause);
    InputComponent->BindAction("FastForward_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::FastForward);
    InputComponent->BindAction("Rewind_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::Rewind);
    InputComponent->BindAction("Restart_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::Restart);
    InputComponent->BindAction("Incr_Timestep_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::IncrTimestep);
    InputComponent->BindAction("Decr_Timestep_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::DecrTimestep);
    // Mute the audio component
    InputComponent->BindAction("Mute_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::ToggleMute);
    InputComponent->BindAction("ToggleGazeHUD_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::ToggleGazeHUD);
}

void ADReyeVRLevel::ToggleGazeHUD()
{
    EgoVehiclePtr->ToggleGazeHUD();
}

void ADReyeVRLevel::ToggleSpectator()
{
    check(Player != nullptr);
    check(EgoVehiclePtr != nullptr);
    UE_LOG(LogTemp, Log, TEXT("Toggled Spectator!"));
    bIsSpectating = !bIsSpectating;
    if (!bIsSpectating)
    {
        // repossess the ego vehicle
        Player->Possess(EgoVehiclePtr);
        // destroy spectator
        check(SpectatorPtr != nullptr);
        SpectatorPtr->Destroy();
        SpectatorPtr = nullptr;
    }
    else
    {
        // spawn at head-position
        const FVector SpawnLocn = EgoVehiclePtr->GetFPSPosn();
        const FRotator SpawnRotn = EgoVehiclePtr->GetFPSRot();
        // create new spectator pawn
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.ObjectFlags |= RF_Transient;
        SpectatorPtr = GetWorld()->SpawnActor<ADReyeVRSpectator>(ADReyeVRSpectator::StaticClass(), // spectator
                                                                 SpawnLocn, SpawnRotn, SpawnParams);
        check(SpectatorPtr != nullptr);
        // possess new spectator pawn
        Player->Possess(SpectatorPtr);
    }
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
    UCarlaStatics::GetRecorder(GetWorld())->RecRewind();
}

void ADReyeVRLevel::Restart()
{
    UE_LOG(LogTemp, Log, TEXT("Restarting recording"));
    UCarlaStatics::GetRecorder(GetWorld())->RecRestart();
}

void ADReyeVRLevel::IncrTimestep()
{
    UE_LOG(LogTemp, Log, TEXT("Incr timestep"));
    UCarlaStatics::GetRecorder(GetWorld())->RecIncrTimestep(0.2);
}

void ADReyeVRLevel::DecrTimestep()
{
    UE_LOG(LogTemp, Log, TEXT("Decr Timestep"));
    UCarlaStatics::GetRecorder(GetWorld())->RecIncrTimestep(-0.2);
}

void ADReyeVRLevel::ToggleMute()
{
    // mute all components with audio
    TArray<AActor *> FoundActors;
    // searching for all AWheeledVehicles instead of ACarlaWheeledVehicles to
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWheeledVehicle::StaticClass(), FoundActors);
    bIsMuted = !bIsMuted;
    for (AActor *A : FoundActors)
    {
        ACarlaWheeledVehicle *C = CastChecked<ACarlaWheeledVehicle>(A);
        if (C != nullptr)
        {
            float NewVolume;
            if (C->IsA(AEgoVehicle::StaticClass()))
            {
                NewVolume = EgoVehicleMaxVolume * (1.f - int(bIsMuted));
            }
            else
            {
                NewVolume = NonEgoMaxVolume * (1.f - int(bIsMuted));
            }
            C->SetVolume(NewVolume);
        }
    }
}