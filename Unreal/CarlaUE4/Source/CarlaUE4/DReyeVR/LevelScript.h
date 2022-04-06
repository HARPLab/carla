#pragma once

/// TODO: clean up circular dependency!
#include "EgoVehicle.h"              // DReyeVR ego vehicle ptr
#include "Engine/LevelScriptActor.h" // ALevelScriptActor
#include "Periph.h"                  // PeriphSystem

#include "LevelScript.generated.h"

class AEgoVehicle;

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
    void SetupSpectator();
    bool FindEgoVehicle();

    // EgoVehicle functions
    enum DRIVER
    {
        HUMAN,
        SPECTATOR,
        AI,
    } ControlMode;
    void PossessEgoVehicle();
    void PossessSpectator();
    void HandoffDriverToAI();

    // Recorder media functions
    void PlayPause();
    void FastForward();
    void Rewind();
    void Restart();
    void IncrTimestep();
    void DecrTimestep();

    // Replayer
    void SetupReplayer();

    // Meta world functions
    void SetVolume();
    FTransform GetSpawnPoint(int SpawnPointIndex = 0) const;

    // Custom actors
    void ReplayCustomActor(const DReyeVR::CustomActorData &RecorderData, const double Per);

    // periph stimuli
    void LegacyReplayPeriph(const DReyeVR::AggregateData &RecorderData, const double Per);

  private:
    // for handling inputs and possessions
    APlayerController *Player = nullptr;
    AController *AI_Player = nullptr;

    // for toggling bw spectator mode
    bool bIsSpectating = true;
    APawn *SpectatorPtr = nullptr;
    AEgoVehicle *EgoVehiclePtr = nullptr;

    // for audio control
    float EgoVolumePercent;
    float NonEgoVolumePercent;
    float AmbientVolumePercent;

    // for recorder/replayer params
    bool bReplaySync = false;        // false allows for interpolation
    bool bRecorderInitiated = false; // allows tick-wise checking for replayer/recorder

    // for the periph stimuli
    PeriphSystem PS;
    FRotator PeriphRotationOffset;
};