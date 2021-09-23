#pragma once

#include "Camera/CameraComponent.h"      // UCameraComponent
#include "Components/InputComponent.h"   // InputComponent
#include "CoreMinimal.h"                 // Unreal functions
#include "GameFramework/SpectatorPawn.h" // ASpectatorPawn

#include "DReyeVRSpectator.generated.h"

UCLASS()
class CARLAUE4_API ADReyeVRSpectator : public ASpectatorPawn
{
    GENERATED_BODY()

  public:
    // Sets default values for this pawn's properties
    ADReyeVRSpectator(const FObjectInitializer &ObjectInitializer);

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

  protected:
    // Called when the game starts (spawned) or ends (destroyed)
    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;

    // World variables
    UWorld *World;
    APlayerController *Player;

  private:
    // Movement Control Functions
    void MoveForward(const float Amnt);
    void MoveRight(const float Amnt);
    void MoveUp(const float Amnt);
    void MouseLookUp(const float Amnt);
    void MouseTurn(const float Amnt);
    void EnableSprintMode();
    void DisableSprintMode();

    bool InvertY = false;       // for mouse controls
    float SprintScale = 1.f;    // when shift pressed, increase movement scale
    float MovementScale = 30.f; // base speed of specator
    float RotationScale = 0.9f; // base rotation speed of spectator
};
