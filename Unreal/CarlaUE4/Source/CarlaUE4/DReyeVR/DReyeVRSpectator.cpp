#include "DReyeVRSpectator.h"
#include "Engine/World.h"           // GetWorld
#include "GameFramework/Actor.h"    // Destroy
#include "Kismet/GameplayStatics.h" // GetPlayerController

// Sets default values
ADReyeVRSpectator::ADReyeVRSpectator(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;

    // don't use the default controls (like spacebar)
    bAddDefaultMovementBindings = false;
}

void ADReyeVRSpectator::BeginPlay()
{
    // Called when the game starts or when spawned
    Super::BeginPlay();

    // Gather information about the world
    World = GetWorld();
    Player = UGameplayStatics::GetPlayerController(World, 0); // main player (0) controller

    UE_LOG(LogTemp, Log, TEXT("Initialize DReyeVR Spectator"));
}

void ADReyeVRSpectator::BeginDestroy()
{
    Super::BeginDestroy();
}

// Called every frame
void ADReyeVRSpectator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

/// ========================================== ///
/// ----------------:INPUTS:------------------ ///
/// ========================================== ///

// Called to bind functionality to input
void ADReyeVRSpectator::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    /// NOTE: to see all DReyeVR inputs see DefaultInput.ini
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // generic WASDEQ 6 axis movement
    PlayerInputComponent->BindAxis("MoveForward", this, &ADReyeVRSpectator::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ADReyeVRSpectator::MoveRight);
    PlayerInputComponent->BindAxis("MoveUp", this, &ADReyeVRSpectator::MoveUp);

    // mouse inputs
    PlayerInputComponent->BindAxis("MouseLookUp_DReyeVR", this, &ADReyeVRSpectator::MouseLookUp);
    PlayerInputComponent->BindAxis("MouseTurn_DReyeVR", this, &ADReyeVRSpectator::MouseTurn);

    // hold shift to go into "sprint mode"
    PlayerInputComponent->BindAction("Sprint_DReyeVR", IE_Pressed, this, &ADReyeVRSpectator::EnableSprintMode);
    PlayerInputComponent->BindAction("Sprint_DReyeVR", IE_Released, this, &ADReyeVRSpectator::DisableSprintMode);

    // increase base movement scale with mouse scroll
    // PlayerInputComponent->BindAxis("MouseWheelAxis", this, &ADReyeVRSpectator::Increase);
}

void ADReyeVRSpectator::MoveForward(const float Amnt)
{
    const FVector ForwardVector = FRotationMatrix(Player->GetControlRotation()).GetScaledAxis(EAxis::X);
    SetActorLocation(GetActorLocation() + SprintScale * Amnt * MovementScale * ForwardVector);
}

void ADReyeVRSpectator::MoveRight(const float Amnt)
{
    const FVector RightVector = FRotationMatrix(Player->GetControlRotation()).GetScaledAxis(EAxis::Y);
    SetActorLocation(GetActorLocation() + SprintScale * Amnt * MovementScale * RightVector);
}

void ADReyeVRSpectator::MoveUp(const float Amnt)
{
    SetActorLocation(GetActorLocation() + SprintScale * Amnt * MovementScale * FVector::UpVector);
}

void ADReyeVRSpectator::MouseLookUp(const float Amnt)
{
    // https://docs.unrealengine.com/en-US/API/Runtime/Engine/GameFramework/ADefaultPawn/LookUpAtRate/index.html
    Super::LookUpAtRate(RotationScale * Amnt);
}

void ADReyeVRSpectator::MouseTurn(const float Amnt)
{
    // https://docs.unrealengine.com/en-US/API/Runtime/Engine/GameFramework/ADefaultPawn/TurnAtRate/index.html
    Super::TurnAtRate(RotationScale * Amnt);
}

void ADReyeVRSpectator::EnableSprintMode()
{
    SprintScale = 2.0f; // 100% faster
}

void ADReyeVRSpectator::DisableSprintMode()
{
    SprintScale = 1.0f; // normal speed
}