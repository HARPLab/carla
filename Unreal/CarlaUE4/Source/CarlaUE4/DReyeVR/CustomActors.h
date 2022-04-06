#pragma once

#include "Carla/Actor/DReyeVRCustomActor.h" // ADReyeVRCustomActor

#include "CustomActors.generated.h"

/// NOTE: include this as part of your custom actor instance class!
#define CREATE_REQUEST_FACTORY_FN(T)                                                                                   \
    static T *RequestNewActor(UWorld *World, const FString &Name)                                                      \
    {                                                                                                                  \
        check(World != nullptr);                                                                                       \
        FActorSpawnParameters SpawnInfo;                                                                               \
        SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;                    \
        T *Actor = World->SpawnActor<T>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);                        \
        Actor->Initialize(Name);                                                                                       \
        return Actor;                                                                                                  \
    }

UCLASS()
class ABall : public ADReyeVRCustomActor
{
    GENERATED_BODY()
  public:
    ABall(const FObjectInitializer &ObjectInitializer);

    CREATE_REQUEST_FACTORY_FN(ABall);
};

UCLASS()
class ACross : public ADReyeVRCustomActor
{
    GENERATED_BODY()
  public:
    ACross(const FObjectInitializer &ObjectInitializer);

    CREATE_REQUEST_FACTORY_FN(ACross);
};

UCLASS()
class APeriphTarget : public ADReyeVRCustomActor
{
    GENERATED_BODY()
  public:
    APeriphTarget(const FObjectInitializer &ObjectInitializer);

    CREATE_REQUEST_FACTORY_FN(APeriphTarget);
};