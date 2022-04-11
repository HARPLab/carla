#pragma once

#include "Carla/Sensor/DReyeVRData.h" // DReyeVR namespace
#include "GameFramework/Actor.h"      // AActor

#include <unordered_map> // std::unordered_map
#include <utility>       // std::pair
#include <vector>        // std::vector

#include "DReyeVRCustomActor.generated.h"

UCLASS()
class CARLA_API ADReyeVRCustomActor : public AActor // abstract class
{
    GENERATED_BODY()

  public:
    ADReyeVRCustomActor(const FObjectInitializer &ObjectInitializer);

    virtual void Tick(float DeltaSeconds) override;

    static ADReyeVRCustomActor *RequestNewActor(UWorld *World, const FString &Name);
    void Enable();
    void Disable();
    bool IsEnabled() const
    {
        return bIsEnabled;
    }

    void Initialize(const FString &Name);

    void SetInternals(const DReyeVR::CustomActorData &In);

    const DReyeVR::CustomActorData &GetInternals() const;

    static std::unordered_map<std::string, class ADReyeVRCustomActor *> ActiveCustomActors;

    // function to dynamically change the material params of the object at runtime
    void ApplyMaterialParams(const std::vector<std::pair<FName, float>> &ScalarParamsIn,
                             const std::vector<std::pair<FName, FLinearColor>> &VectorParamIn,
                             const int MaterialIdx = 0);

  protected:
    void BeginPlay() override;
    void BeginDestroy() override;
    bool bIsEnabled = false; // initially disabled

    void AssignSM(const FString &Path);
    void AssignMat(const int MatIdx, const FString &Path);

    class DReyeVR::CustomActorData Internals;

    UPROPERTY(EditAnywhere, Category = "Mesh")
    class UStaticMeshComponent *ActorMesh = nullptr;
    static int AllMeshCount;

    // for dynamic (parametrized) material
    std::vector<std::pair<FName, float>> ScalarParams;
    std::vector<std::pair<FName, FLinearColor>> VectorParams;
};