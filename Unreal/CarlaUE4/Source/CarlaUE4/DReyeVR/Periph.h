#pragma once

#include "Carla/Actor/DReyeVRCustomActor.h" // ADReyeVRCustomActor
#include "CustomActors.h"                   // APeriphTarget, ACross

class PeriphSystem
{
  public:
    PeriphSystem();
    void ReadConfigVariables();

    class APeriphTarget *PeriphTarget = nullptr;
    class ACross *Cross = nullptr;

    void Initialize(class UWorld *World);
    void Tick(float DeltaTime, bool bIsReplaying, bool bInCleanRoomExperiment, const UCameraComponent *Camera);
    void LegacyTick(const DReyeVR::LegacyPeriphDataStruct &LegacyData);

  private:
    class UWorld *World = nullptr;

    ///////////////:PERIPH:////////////////
    FRotator PeriphRotator, PeriphRotationOffset;
    FVector2D PeriphYawBounds, PeriphPitchBounds;
    float MaxTimeBetweenFlash;
    float MinTimeBetweenFlash;
    float FlashDuration;
    float PeriphTargetSize, FixationCrossSize;
    float TargetRenderDistance;
    float LastPeriphTick = 0.f;
    float TimeSinceLastFlash = 0.f;
    float NextPeriphTrigger = 0.f;
    bool bUsePeriphTarget = false;
    bool bUseFixedCross = false;
    const FString PeriphTargetName = "PeriphTarget";
    const FString PeriphFixationName = "PeriphCross";

    // Fixation cross movement variables
    FVector FixCrossLoc, CrossVector, HeadNeutralLoc;
    FVector2D TimeBetweenFlashFC, TimeFlashToMoveFC, FCYawBounds, FCPitchBounds;
    FRotator FixCrossRot, HeadNeutralRot; // do we even need to rotate it
    bool FCMovesWHead;
    float FixCrossPitchExtension;
    float FixationMoveTimeOffset;
    float TimeBetweenFCMove;
    float TimeSinceLastFCMove = 0.f;
    float NextFCMove = 0.f;
    float FCMoveMaxRadius;
    int NumFCMoves = 0;
    

  
};
