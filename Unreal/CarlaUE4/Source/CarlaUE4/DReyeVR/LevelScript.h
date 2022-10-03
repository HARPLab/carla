#pragma once

#include "Carla/Actor/DReyeVRCustomActor.h" // ADReyeVRCustomActor
#include "Carla/Sensor/DReyeVRData.h"       // DReyeVR::
#include "Engine/LevelScriptActor.h"        // ALevelScriptActor
#include <unordered_map>                    // std::unordered_map

#include "LevelScript.generated.h"

class AEgoVehicle;
class ADReyeVRPawn;

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

  private:
    // for handling inputs and possessions
    APlayerController *Player = nullptr;
    void StartDReyeVRPawn();
    ADReyeVRPawn *DReyeVR_Pawn = nullptr;

    // for toggling bw spectator mode
    bool bIsSpectating = true;
    APawn *SpectatorPtr = nullptr;
    AEgoVehicle *EgoVehiclePtr = nullptr;

    // for accessing all actors (vehicles/walkers only) in the world
    float RefreshActorSearchTick = 5.f; // tickrate (seconds) for UGameplayStatics::GetAllActorsOfClass
    float TimeSinceLastActorRefresh = 0.f;

    struct ActorAndMetadata
    {
        AActor *Actor;
        FVector BBox_Offset, BBox_Extent;
    };
    std::unordered_map<std::string, ActorAndMetadata> AllActors = {};
    void RefreshActors(float DeltaSeconds);
    void DrawBBoxes();
    std::unordered_map<std::string, ADReyeVRCustomActor *> BBoxes;

    // for audio control
    float EgoVolumePercent;
    float NonEgoVolumePercent;
    float AmbientVolumePercent;

    // for recorder/replayer params
    bool bReplaySync = false;        // false allows for interpolation
    bool bRecorderInitiated = false; // allows tick-wise checking for replayer/recorder
};