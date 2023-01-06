#pragma once

#include "Carla/Actor/DReyeVRCustomActor.h" // ADReyeVRCustomActor
#include "Carla/Game/CarlaGameModeBase.h"   // ACarlaGameModeBase
#include "Carla/Sensor/DReyeVRData.h"       // DReyeVR::
#include <unordered_map>                    // std::unordered_map

#include "DReyeVRGameMode.generated.h"

class AEgoVehicle;
class ADReyeVRPawn;

UCLASS()
class ADReyeVRGameMode : public ACarlaGameModeBase
{
    GENERATED_UCLASS_BODY()

  public:
    ADReyeVRGameMode();

    virtual void BeginPlay() override;

    virtual void BeginDestroy() override;

    virtual void Tick(float DeltaSeconds) override;

    // input & world handling
    void SetupPlayerInputComponent();
    void SetupSpectator();
    bool SetupEgoVehicle();

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
    void DrawBBoxes();
    std::unordered_map<std::string, ADReyeVRCustomActor *> BBoxes;

  private:
    // for handling inputs and possessions
    void StartDReyeVRPawn();
    class APlayerController *Player = nullptr;
    class ADReyeVRPawn *DReyeVR_Pawn = nullptr;
    class UClass *EgoVehicleBPClass = nullptr;

    // for toggling bw spectator mode
    bool bIsSpectating = true;
    class APawn *SpectatorPtr = nullptr;
    class AEgoVehicle *EgoVehiclePtr = nullptr;

    // for audio control
    float EgoVolumePercent;
    float NonEgoVolumePercent;
    float AmbientVolumePercent;

    bool bDoSpawnEgoVehicleTransform = false; // whether or not to use provided SpawnEgoVehicleTransform
    FTransform SpawnEgoVehicleTransform;

    // for recorder/replayer params
    bool bReplaySync = false;        // false allows for interpolation
    bool bRecorderInitiated = false; // allows tick-wise checking for replayer/recorder
};