#pragma once

#include "DReyeVRSpectator.h"        // ADReyeVRSpectator ptr
#include "EgoVehicle.h"              // DReyeVR ego vehicle ptr
#include "Engine/LevelScriptActor.h" // Parent for LevelBP

#include "DReyeVRLevel.generated.h"

UCLASS()
class ADReyeVRLevel : public ALevelScriptActor
{
    GENERATED_UCLASS_BODY()

  public:
    ADReyeVRLevel();

    virtual void BeginPlay() override;

    virtual void BeginDestroy() override;

    virtual void Tick(float DeltaSeconds) override;

    // input handling
    void SetupPlayerInputComponent();

    // util functions
    // EgoVehicle functions
    void ToggleSpectator();

    // Recorder media functions
    void PlayPause();
    void FastForward();
    void Rewind();
    void Restart();
    void IncrTimestep();
    void DecrTimestep();

    // Meta world functions
    void ToggleMute();
    void ToggleGazeHUD();

  private:
    // for handling inputs and possessions
    APlayerController *Player = nullptr;

    // for toggling bw spectator mode
    bool bIsSpectating = false;
    ADReyeVRSpectator *SpectatorPtr = nullptr;
    AEgoVehicle *EgoVehiclePtr = nullptr;

    // for muting all the world sounds
    bool bIsMuted = false;
    const float EgoVehicleMaxVolume = 0.8f;
    const float NonEgoMaxVolume = 0.4f;
};