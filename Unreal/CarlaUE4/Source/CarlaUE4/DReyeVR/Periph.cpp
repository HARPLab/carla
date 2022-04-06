#include "Periph.h"
#include "DReyeVRUtils.h" // ReadConfigValue

PeriphSystem::PeriphSystem()
{
    ReadConfigVariables();
}

void PeriphSystem::ReadConfigVariables()
{
    // peripheral target
    ReadConfigValue("PeripheralTarget", "EnablePeriphTarget", bUsePeriphTarget);
    ReadConfigValue("PeripheralTarget", "YawBounds", PeriphYawBounds);
    ReadConfigValue("PeripheralTarget", "PitchBounds", PeriphPitchBounds);
    ReadConfigValue("PeripheralTarget", "RotationOffset", PeriphRotationOffset);
    ReadConfigValue("PeripheralTarget", "MaxTimeBetweenFlashSec", MaxTimeBetweenFlash);
    ReadConfigValue("PeripheralTarget", "MinTimeBetweenFlashSec", MinTimeBetweenFlash);
    ReadConfigValue("PeripheralTarget", "FlashDurationSec", FlashDuration);
    ReadConfigValue("PeripheralTarget", "TargetRadius", PeriphTargetRadius);
    ReadConfigValue("PeripheralTarget", "TargetRenderDistanceM", TargetRenderDistance);
    ReadConfigValue("PeripheralTarget", "EnableFixedCross", bUseFixedCross);
}

void PeriphSystem::Initialize(class UWorld *WorldIn)
{
    World = WorldIn;
    check(World != nullptr);
    if (bUseFixedCross)
    {
        Cross = ACross::RequestNewActor(World, PeriphFixationName);
        Cross->SetActorScale3D(0.1f * FVector::OneVector);
        check(Cross != nullptr);
    }
    if (bUsePeriphTarget)
    {
        PeriphTarget = APeriphTarget::RequestNewActor(World, PeriphTargetName);
        PeriphTarget->SetActorScale3D(PeriphTargetRadius * FVector::OneVector);
        check(PeriphTarget != nullptr);
    }
}

void PeriphSystem::Tick(float DeltaTime, bool bIsReplaying, bool bInCleanRoomExperiment, const UCameraComponent *Camera)
{
    // if replaying, don't tick periph system
    if (bIsReplaying || World == nullptr)
    {
        // replay of legacy periph target is handled in UpdateData
        if (PeriphTarget != nullptr)
            PeriphTarget->Disable();
        if (Cross != nullptr)
            Cross->Disable();
        return;
    }

    const FVector &CameraLoc = Camera->GetComponentLocation();
    const FRotator &CameraRot = Camera->GetComponentRotation();
    if (bUseFixedCross)
    {
        if (bInCleanRoomExperiment)
        {
            const FVector CrossVector = CameraRot.RotateVector(FVector::ForwardVector);
            Cross->SetActorLocation(CameraLoc + CrossVector * TargetRenderDistance * 100.f);
            Cross->SetActorRotation(CameraRot);
            Cross->Enable();
        }
        else
        {
            Cross->Disable();
        }
    }

    if (bUsePeriphTarget)
    {
        // generate stimuli every TimeBetweenFlash second chunks, and log that time
        if (TimeSinceLastFlash < MaxTimeBetweenFlash + FlashDuration)
        {
            if (TimeSinceLastFlash == 0.f)
            {
                // update time for the next periph trigger
                NextPeriphTrigger = FMath::RandRange(MinTimeBetweenFlash, MaxTimeBetweenFlash);

                // generate random position for the next periph target
                float RandYaw = FMath::RandRange(PeriphYawBounds.X, PeriphYawBounds.Y);
                float RandPitch = FMath::RandRange(PeriphPitchBounds.X, PeriphPitchBounds.Y);
                float Roll = 0.f;
                PeriphRotator = FRotator(RandPitch, RandYaw, Roll);
            }
            else if (LastPeriphTick <= NextPeriphTrigger && TimeSinceLastFlash > NextPeriphTrigger)
            {
                // turn on periph target
                PeriphTarget->Enable();
                UE_LOG(LogTemp, Log, TEXT("Periph Target On @ %f"), UGameplayStatics::GetRealTimeSeconds(World));
            }
            else if (LastPeriphTick <= NextPeriphTrigger + FlashDuration &&
                     TimeSinceLastFlash > NextPeriphTrigger + FlashDuration)
            {
                // turn off periph target
                PeriphTarget->Disable();
                UE_LOG(LogTemp, Log, TEXT("Periph Target Off @ %f"), UGameplayStatics::GetRealTimeSeconds(World));
            }
            LastPeriphTick = TimeSinceLastFlash;
            TimeSinceLastFlash += DeltaTime;
        }
        else
        {
            // reset the periph flashing
            TimeSinceLastFlash = 0.f;
        }
        if (PeriphTarget->IsEnabled())
        {
            const FVector PeriphFinal = CameraRot.RotateVector((PeriphRotationOffset + PeriphRotator).Vector());
            PeriphTarget->SetActorLocation(CameraLoc + PeriphFinal * TargetRenderDistance * 100.f);
        }
    }

// Draw debug border markers
#if WITH_EDITOR
    const FVector TopLeft =
        CameraRot.RotateVector((PeriphRotationOffset + FRotator(PeriphPitchBounds.X, PeriphYawBounds.X, 0.f)).Vector());
    const FVector TopRight =
        CameraRot.RotateVector((PeriphRotationOffset + FRotator(PeriphPitchBounds.X, PeriphYawBounds.Y, 0.f)).Vector());
    const FVector BotLeft =
        CameraRot.RotateVector((PeriphRotationOffset + FRotator(PeriphPitchBounds.Y, PeriphYawBounds.X, 0.f)).Vector());
    const FVector BotRight =
        CameraRot.RotateVector((PeriphRotationOffset + FRotator(PeriphPitchBounds.Y, PeriphYawBounds.Y, 0.f)).Vector());
    DrawDebugSphere(World, CameraLoc + TopLeft * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
    DrawDebugSphere(World, CameraLoc + TopRight * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
    DrawDebugSphere(World, CameraLoc + BotLeft * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
    DrawDebugSphere(World, CameraLoc + BotRight * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
#endif
}