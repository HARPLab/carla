#include "DReyeVRLevel.h"
#include "Carla/Game/CarlaStatics.h"           // UCarlaStatics::GetRecorder
#include "Carla/Sensor/DReyeVRSensor.h"        // ADReyeVRSensor
#include "Carla/Vehicle/CarlaWheeledVehicle.h" // ACarlaWheeledVehicle
#include "Components/InputComponent.h" // BindKey (Also needs "SlateCore" & "Slate" in PublicDependencyModuleNames)
#include "EgoVehicle.h"                // AEgoVehicle
#include "Kismet/GameplayStatics.h"    // GetPlayerController

ADReyeVRLevel::ADReyeVRLevel()
{
    // initialize stuff here
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;
}

ADReyeVRLevel::ADReyeVRLevel(FObjectInitializer const &FO) : ADReyeVRLevel()
{
    // initialize stuff here
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;
}

bool ADReyeVRLevel::FindEgoVehicle()
{
    if (EgoVehiclePtr != nullptr)
        return true;
    TArray<AActor *> FoundEgoVehicles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEgoVehicle::StaticClass(), FoundEgoVehicles);
    if (FoundEgoVehicles.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Found EgoVehicle"));
        /// TODO: add support for multiple Ego Vehicles?
        EgoVehiclePtr = Cast<AEgoVehicle>(FoundEgoVehicles[0]);
        EgoVehiclePtr->SetLevel(this);
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
    SetActorTickEnabled(false); // make sure we do not tick ourselves

    // enable input tracking
    InputEnabled();

    // start input mapping
    SetupPlayerInputComponent();

    // Initialize DReyeVR spectator
    SetupSpectator();

    /// TODO: optionally, spawn ego-vehicle here with parametrized inputs

    // Find the ego vehicle in the world
    FindEgoVehicle();

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

void ADReyeVRLevel::SetupSpectator()
{
    SpectatorPtr = UCarlaStatics::GetCurrentEpisode(GetWorld())->GetSpectatorPawn();
}

void ADReyeVRLevel::BeginDestroy()
{
    Super::BeginDestroy();
    UE_LOG(LogTemp, Log, TEXT("Finished Level"));
}

void ADReyeVRLevel::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (EgoVehiclePtr && SpectatorPtr && AI_Player)
    {
        if (ControlMode == DRIVER::AI) // when AI is controlling EgoVehicle
        {
            // Physically attach the Controller to the specified Pawn, so that our position reflects theirs.
            // const FVector &NewVehiclePosn = EgoVehiclePtr->GetNextCameraPosn(DeltaSeconds); // for pre-physics tick
            const FVector &NewVehiclePosn = EgoVehiclePtr->GetCameraPosn(); // for post-physics tick
            SpectatorPtr->SetActorLocationAndRotation(NewVehiclePosn, EgoVehiclePtr->GetCameraRot());
        }
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
    // Mute the audio component
    InputComponent->BindAction("Mute_DReyeVR", IE_Pressed, this, &ADReyeVRLevel::ToggleMute);
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