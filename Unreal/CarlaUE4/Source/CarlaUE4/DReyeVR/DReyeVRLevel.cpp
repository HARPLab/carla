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

bool ADReyeVRLevel::FindEgoVehicle()
{
    if (EgoVehiclePtr != nullptr)
        return true;
    TArray<AActor *> FoundEgoVehicles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEgoVehicle::StaticClass(), FoundEgoVehicles);
    if (FoundEgoVehicles.Num() > 0)
    {
        EgoVehiclePtr = Cast<AEgoVehicle>(FoundEgoVehicles[0]);
        UE_LOG(LogTemp, Log, TEXT("Found EgoVehicle"));
        if (!AI_Player)
            AI_Player = EgoVehiclePtr->GetController();
        return true;
    }
    UE_LOG(LogTemp, Log, TEXT("Did not find EgoVehicle"));
    return false;
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

    // Initialize DReyeVR spectator
    SetupSpectator();

    /// TODO: optionally, spawn ego-vehicle here with parametrized inputs

    // Find the ego vehicle in the world
    FindEgoVehicle();
}

void ADReyeVRLevel::SetupSpectator()
{
    SpectatorPtr = UCarlaStatics::GetCurrentEpisode(GetWorld())->GetSpectatorPawn();
    UE_LOG(LogTemp, Log, TEXT("Set up spectator"));
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
    // InputComponent->BindAction("ToggleCamera", IE_Pressed, this, &ADReyeVRLevel::ToggleSpectator);
    InputComponent->BindAction("PlayPause_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::PlayPause);
    InputComponent->BindAction("FastForward_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::FastForward);
    InputComponent->BindAction("Rewind_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::Rewind);
    InputComponent->BindAction("Restart_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::Restart);
    InputComponent->BindAction("Incr_Timestep_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::IncrTimestep);
    InputComponent->BindAction("Decr_Timestep_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::DecrTimestep);
    // Mute the audio component
    InputComponent->BindAction("Mute_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::ToggleMute);
    /// TODO: refactor
    // InputComponent->BindAction("ToggleGazeHUD_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::ToggleGazeHUD);
    // Driver Handoff examples
    InputComponent->BindKey(EKeys::One, IE_Pressed, this, &ADReyeVRLevel::PossessEgoVehicle);
    InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ADReyeVRLevel::PossessSpectator);
    InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ADReyeVRLevel::HandoffDriverToAI);
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
    UE_LOG(LogTemp, Log, TEXT("Successful handoff to human driver"));
}

void ADReyeVRLevel::PossessSpectator()
{
    // check if already possessing spectator
    if (Player->GetPawn() == SpectatorPtr)
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
        const FVector &EgoLocn = EgoVehiclePtr->GetFPSPosn();
        const FRotator &EgoRotn = EgoVehiclePtr->GetFPSRot();
        SpectatorPtr->SetActorLocationAndRotation(EgoLocn, EgoRotn);
    }
    // repossess the ego vehicle
    Player->Possess(SpectatorPtr);
    UE_LOG(LogTemp, Log, TEXT("Switching to Spectator player"));
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
    // PossessSpectator();
    AI_Player->Possess(EgoVehiclePtr);
    UE_LOG(LogTemp, Log, TEXT("Successful handoff to AI driver"));
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
    // searching for all AWheeledVehicles instead of ACarlaWheeledVehicles to mute them too
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