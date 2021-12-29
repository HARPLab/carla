#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Sensor/Sensor.h"
#include "DReyeVRSensorData.h" // SensorData struct
#include <cstdint>             // int64_t
#include <string>

#include "DReyeVRSensor.generated.h"

class UCarlaEpisode;

UCLASS()
class CARLA_API ADReyeVRSensor : public ASensor
{
    GENERATED_BODY()

  public:
    ADReyeVRSensor(const FObjectInitializer &ObjectInitializer);

    static FActorDefinition GetSensorDefinition();

    void Set(const FActorDescription &ActorDescription) override;

    void SetOwner(AActor *Owner) override;

    virtual void PostPhysTick(UWorld *W, ELevelTick TickType, float DeltaSeconds) override;

    // everything stored in the sensor is held in this struct
    /// TODO: make this non-static and use a smarter scheme for cross-class communication
    static class DReyeVR::SensorData *Data;

    class DReyeVR::SensorData *GetData()
    {
        return ADReyeVRSensor::Data;
    }

    const class DReyeVR::SensorData *GetData() const
    {
        // read-only variant of GetData
        return ADReyeVRSensor::Data;
    }

    static void UpdateReplayData(const class DReyeVR::SensorData &RecorderData, const FTransform &EgoTransform,
                                 const double Per);

    static bool bIsReplaying;
    static FTransform EgoReplayTransform;
    static float EgoReplayVelocity;

  protected:
    void BeginPlay() override;
    void BeginDestroy() override;

    UWorld *World; // to get info about the world: time, frames, etc.

    bool StreamSensorData = true;

    static void InterpPositionAndRotation(const FVector &Pos1, const FRotator &Rot1, const FVector &Pos2,
                                          const FRotator &Rot2, const double Per, FVector &Location,
                                          FRotator &Rotation);
};