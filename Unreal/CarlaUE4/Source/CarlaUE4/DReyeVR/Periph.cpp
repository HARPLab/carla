#include "Periph.h"
#include "DReyeVRUtils.h" // ReadConfigValue
#include "Kismet/KismetMathLibrary.h"   // FindLookAtRotation

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
    ReadConfigValue("PeripheralTarget", "PeriphTargetSize", PeriphTargetSize);
    // fixation cross params
    ReadConfigValue("PeripheralTarget", "EnableFixedCross", bUseFixedCross);
    ReadConfigValue("PeripheralTarget", "FixationCrossSize", FixationCrossSize);
    
    ReadConfigValue("CleanRoom", "FixCrossPitchExtension", FixCrossPitchExtension);
    ReadConfigValue("CleanRoom", "FCYawBounds", FCYawBounds);
    ReadConfigValue("CleanRoom", "FCPitchBounds", FCPitchBounds);
    ReadConfigValue("CleanRoom", "TimeBetweenFlashFC", TimeBetweenFlashFC);
    ReadConfigValue("CleanRoom", "FixationMoveTimeOffset", FixationMoveTimeOffset);
    ReadConfigValue("CleanRoom", "TimeBetweenFCMove", TimeBetweenFCMove);
    ReadConfigValue("CleanRoom", "FCMoveMaxRadius", FCMoveMaxRadius);
    ReadConfigValue("CleanRoom", "TimeFlashToMoveFC", TimeFlashToMoveFC);
    ReadConfigValue("CleanRoom", "FCMovesWHead", FCMovesWHead);
    

    // general
    ReadConfigValue("PeripheralTarget", "TargetRenderDistanceM", TargetRenderDistance);
}

void PeriphSystem::Initialize(class UWorld *WorldIn)
{
    World = WorldIn;
    check(World != nullptr);
    if (bUseFixedCross)
    {
        Cross = ACross::RequestNewActor(World, PeriphFixationName);
        Cross->SetActorScale3D(FixationCrossSize * FVector::OneVector);
        check(Cross != nullptr);
    }
    if (bUsePeriphTarget)
    {
        PeriphTarget = APeriphTarget::RequestNewActor(World, PeriphTargetName);
        PeriphTarget->SetActorScale3D(PeriphTargetSize * FVector::OneVector);
        check(PeriphTarget != nullptr);
    }
}

void PeriphSystem::Tick(float DeltaTime, bool bIsReplaying, bool bInCleanRoomExperiment, const UCameraComponent *Camera)
{
    // if replaying, don't tick periph system
    if (bIsReplaying || World == nullptr)
    {
        // replay of legacy periph target is handled in UpdateData
        if (PeriphTarget != nullptr && PeriphTarget->IsEnabled())
            PeriphTarget->Disable();
        if (Cross != nullptr && Cross->IsEnabled())
            Cross->Disable();
        return;
    }

    const FVector &CameraLoc = Camera->GetComponentLocation();
    const FRotator &CameraRot = Camera->GetComponentRotation();
    if (bUseFixedCross)
    {
        if (bInCleanRoomExperiment)
        {   
            Cross->Enable();
            // TODO may not need this special case
            if (NumFCMoves == 0)
            {
                CrossVector = CameraRot.RotateVector(FVector::ForwardVector);
                FixCrossLoc = CameraLoc + CrossVector * TargetRenderDistance * 100.f;
                Cross->SetActorLocation(FixCrossLoc);
                Cross->SetActorRotation(CameraRot);                

                // init the bounds for the FC Location
                HeadNeutralLoc = CameraLoc; HeadNeutralRot = CameraRot;
            }
            
            // if (FCReset)
            // {
            if (TimeSinceLastFCMove == 0.f)
            {
                // update time for the next periph trigger -- this is basically the OFD
                NextPeriphTrigger = FMath::RandRange(TimeBetweenFlashFC.X, TimeBetweenFlashFC.Y) // make sure you get the float version of this fn
                +  FixationMoveTimeOffset;   // it takes some non zero time to start a new fixation 
                UE_LOG(LogTemp, Log, TEXT("Next periph target gen \t @ %f"), NextPeriphTrigger);

                //TODO decide where periph target is triggered                                        
                // generate random position for the next periph target
                const float RandYaw = FMath::RandRange(PeriphYawBounds.X, PeriphYawBounds.Y);
                const float RandPitch = FMath::RandRange(PeriphPitchBounds.X, PeriphPitchBounds.Y);
                const float Roll = 0.f;
                PeriphRotator = FRotator(RandPitch, RandYaw, Roll);
            }
            else if (LastPeriphTick <= NextPeriphTrigger && TimeSinceLastFlash > NextPeriphTrigger)
            {
                // turn on periph target
                PeriphTarget->Enable();
                UE_LOG(LogTemp, Log, TEXT("Periph Target On \t @ %f"), UGameplayStatics::GetRealTimeSeconds(World));
            }
            else if (LastPeriphTick <= NextPeriphTrigger + FlashDuration
                        && TimeSinceLastFlash > NextPeriphTrigger + FlashDuration)
            {
                // turn off periph target
                PeriphTarget->Disable();
                UE_LOG(LogTemp, Log, TEXT("Periph Target On \t @ %f"), UGameplayStatics::GetRealTimeSeconds(World));
                TimeSinceLastFlash = 0.f;

                NextFCMove = TimeSinceLastFCMove + DeltaTime
                    + FMath::RandRange(TimeFlashToMoveFC.X, TimeFlashToMoveFC.Y);
            }
            else if (LastPeriphTick > NextPeriphTrigger + FlashDuration 
                    && TimeSinceLastFCMove > NextFCMove)
            {
                UE_LOG(LogTemp, Log, TEXT("Moving the FC \t @ %f"), UGameplayStatics::GetRealTimeSeconds(World));
                                
                
                const float RandYaw = FMath::RandRange(FCYawBounds.X, FCYawBounds.Y);
                const float RandPitch = FMath::RandRange(FCPitchBounds.X, FCPitchBounds.Y);
                const float Roll = 0.f;
                CrossVector = FRotator(RandYaw, RandYaw, Roll).Vector();
                // float FCcoeff = FMath::FRandRange(-1, 1);
                // CrossVector = FVector::ForwardVector + FCcoeff*FVector::LeftVector + (FCcoeff/1.5)*FVector::UpVector;

                //TODO this should not be camera relative, maybe neutral pos relative
                FixCrossLoc = HeadNeutralLoc + HeadNeutralRot.RotateVector(CrossVector) * TargetRenderDistance * 100.f;
                // FixCrossLoc = CameraLoc + CameraRot.RotateVector(CrossVector) * TargetRenderDistance * 100.f;
                
                Cross->SetActorLocation(FixCrossLoc); // if no head movement compensation for FC, this is where loc is set
                // adding DeltaTime below makes these reset 0 and triggers first if case above
                TimeSinceLastFCMove = -DeltaTime; 
                TimeSinceLastFlash = -DeltaTime;
                NumFCMoves++;
            }

            LastPeriphTick = TimeSinceLastFCMove;
            TimeSinceLastFlash += DeltaTime;
            TimeSinceLastFCMove += DeltaTime;
            
            // if (FCMovesWHead)
            if (FCMovesWHead)
            {
                FixCrossLoc = CameraLoc + CameraRot.RotateVector(CrossVector) * TargetRenderDistance * 100.f;
                Cross->SetActorLocation(FixCrossLoc);                
            }

            // TODO this is not right, it should be along the line joining 
            // const FRotator LookAtCamRot = (CameraLoc - FixCrossLoc).Rotation();
            const FRotator LookAtCamRot = UKismetMathLibrary::FindLookAtRotation(FixCrossLoc, CameraLoc);
            FixCrossRot = LookAtCamRot; // this is where the FC should be facing to face the first person cam always
            Cross->SetActorRotation(LookAtCamRot);



            if (PeriphTarget->IsEnabled())
            {
                const FVector PeriphFinal = -FixCrossRot.RotateVector((PeriphRotator).Vector());
                PeriphTarget->SetActorLocation(CameraLoc + PeriphFinal * TargetRenderDistance * 100.f);
            }

            
        }
        else if (Cross->IsEnabled())
        {
            Cross->Disable();
        }
    }

    if (bUsePeriphTarget && !bInCleanRoomExperiment)
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
                UE_LOG(LogTemp, Log, TEXT("Periph Target On \t @ %f"), UGameplayStatics::GetRealTimeSeconds(World));
            }
            else if (LastPeriphTick <= NextPeriphTrigger + FlashDuration &&
                     TimeSinceLastFlash > NextPeriphTrigger + FlashDuration)
            {
                // turn off periph target
                PeriphTarget->Disable();
                UE_LOG(LogTemp, Log, TEXT("Periph Target Off \t @ %f"), UGameplayStatics::GetRealTimeSeconds(World));
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
FVector TopLeft, TopRight, BotLeft, BotRight;
if (!bInCleanRoomExperiment)
{
    TopLeft =
        CameraRot.RotateVector((PeriphRotationOffset 
        + FRotator(PeriphPitchBounds.X, PeriphYawBounds.X, 0.f)).Vector());
    TopRight =
        CameraRot.RotateVector((PeriphRotationOffset 
        + FRotator(PeriphPitchBounds.X, PeriphYawBounds.Y, 0.f)).Vector());
    BotLeft =
        CameraRot.RotateVector((PeriphRotationOffset 
        + FRotator(PeriphPitchBounds.Y, PeriphYawBounds.X, 0.f)).Vector());
    BotRight =
        CameraRot.RotateVector((PeriphRotationOffset 
        + FRotator(PeriphPitchBounds.Y, PeriphYawBounds.Y, 0.f)).Vector());
    DrawDebugSphere(World, CameraLoc + TopLeft * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
    DrawDebugSphere(World, CameraLoc + TopRight * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
    DrawDebugSphere(World, CameraLoc + BotLeft * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
    DrawDebugSphere(World, CameraLoc + BotRight * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
}
else
{
    TopLeft =
        -FixCrossRot.RotateVector((FRotator(PeriphPitchBounds.X-FixCrossPitchExtension,
         PeriphYawBounds.X, 0.f)).Vector());
    TopRight =
        -FixCrossRot.RotateVector((FRotator(PeriphPitchBounds.X-FixCrossPitchExtension,
         PeriphYawBounds.Y, 0.f)).Vector());
    BotLeft =
        -FixCrossRot.RotateVector((FRotator(PeriphPitchBounds.Y+FixCrossPitchExtension,
         PeriphYawBounds.X, 0.f)).Vector());
    BotRight =
        -FixCrossRot.RotateVector((FRotator(PeriphPitchBounds.Y+FixCrossPitchExtension,
         PeriphYawBounds.Y, 0.f)).Vector());
    DrawDebugSphere(World, CameraLoc + TopLeft * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
    DrawDebugSphere(World, CameraLoc + TopRight * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
    DrawDebugSphere(World, CameraLoc + BotLeft * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
    DrawDebugSphere(World, CameraLoc + BotRight * TargetRenderDistance * 100.f, 4.0f, 12, FColor::Blue);
}
#endif
}