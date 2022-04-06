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
    ReadConfigValue("PeripheralTarget", "EnableFixedCrosshair", bUseFixedCrosshair);
}

void PeriphSystem::Initialize(class UWorld *WorldIn)
{
    World = WorldIn;
    check(World != nullptr);
}

void PeriphSystem::Tick(float DeltaTime, bool bIsReplaying, bool bInCleanRoomExperiment, const UCameraComponent *Camera)
{
    // if replaying, don't tick periph system
    if (bIsReplaying || World == nullptr)
    {
        // replay of legacy periph target is handled in UpdateData
        if (PeriphTarget != nullptr)
            ADReyeVRCustomActor::RequestDestroy(PeriphTarget);
        if (Crosshair != nullptr)
            ADReyeVRCustomActor::RequestDestroy(Crosshair);
        return;
    }

    const FVector &CameraLoc = Camera->GetComponentLocation();
    const FRotator &CameraRot = Camera->GetComponentRotation();
    if (bUseFixedCrosshair)
    {
        if (bInCleanRoomExperiment)
        {
            if (Crosshair == nullptr)
                Crosshair = ACross::RequestNewActor(World, PeriphFixationName);
            const FVector CrosshairVector = CameraRot.RotateVector(FVector::ForwardVector);
            Crosshair->SetActorLocation(CameraLoc + CrosshairVector * TargetRenderDistance * 100.f);
            Crosshair->SetActorRotation(CameraRot);
            Crosshair->SetActorScale3D(0.1f * FVector::OneVector);
        }
        else
        {
            if (Crosshair != nullptr)
                ADReyeVRCustomActor::RequestDestroy(Crosshair);
        }
    }

    if (!bUsePeriphTarget)
        return;

    // generate stimuli every TimeBetweenFlash second chunks, and log that time
    /// TODO: all these magic numbers need to be parameterized
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
            if (PeriphTarget != nullptr)
                ADReyeVRCustomActor::RequestDestroy(PeriphTarget);
            PeriphTarget = APeriphTarget::RequestNewActor(World, PeriphName);
            UE_LOG(LogTemp, Log, TEXT("Periph Target On @ %f"), UGameplayStatics::GetRealTimeSeconds(World));
        }
        else if (LastPeriphTick <= NextPeriphTrigger + FlashDuration &&
                 TimeSinceLastFlash > NextPeriphTrigger + FlashDuration)
        {
            // turn off periph target
            if (PeriphTarget == nullptr)
                PeriphTarget = APeriphTarget::RequestNewActor(World, PeriphName);
            ADReyeVRCustomActor::RequestDestroy(PeriphTarget);
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

    if (PeriphTarget != nullptr)
    {
        const FVector PeriphFinal = CameraRot.RotateVector((PeriphRotationOffset + PeriphRotator).Vector());
        PeriphTarget->SetActorLocation(CameraLoc + PeriphFinal * TargetRenderDistance * 100.f);
        PeriphTarget->SetActorScale3D(PeriphTargetRadius * FVector::OneVector);
    }
}