#include "Periph.h"
#include "DReyeVRUtils.h" // ReadConfigValue
#include "Kismet/KismetMathLibrary.h"   // FindLookAtRotation, Acos

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
    ReadConfigValue("CleanRoom", "PeriphSpawnRatio", PeriphSpawnRatio);

    // general
    ReadConfigValue("PeripheralTarget", "TargetRenderDistanceM", TargetRenderDistance);
}

void PeriphSystem::Initialize(class UWorld *WorldIn)
{
    World = WorldIn;
    check(World != nullptr);
    if (bUseFixedCross)
    {
        Cross = ADReyeVRCustomActor::CreateNew(SM_CROSS, MAT_OPAQUE, World, PeriphFixationName);
        check(Cross != nullptr);
        Cross->SetActorScale3D(FixationCrossSize * FVector::OneVector);
        Cross->MaterialParams.BaseColor = FLinearColor::Black;
        Cross->MaterialParams.Emissive = 0 * FLinearColor::Black;
    }
    if (bUsePeriphTarget)
    {
        PeriphTarget = ADReyeVRCustomActor::CreateNew(SM_SPHERE, MAT_OPAQUE, World, PeriphTargetName);
        check(PeriphTarget != nullptr);
        PeriphTarget->SetActorScale3D(PeriphTargetSize * FVector::OneVector);
        float Emissive;
        ReadConfigValue("PeripheralTarget", "EmissionFactor", Emissive);
        PeriphTarget->MaterialParams.Emissive = Emissive * FLinearColor::Red;
        bool bUseLegacyPeriphColour;
        ReadConfigValue("PeripheralTarget", "UseLegacyPeriphColour", bUseLegacyPeriphColour);
        if (bUseLegacyPeriphColour)
        {
            PeriphTarget->MaterialParams.Metallic = 0.f;
            PeriphTarget->MaterialParams.Specular = 0.5f;
            PeriphTarget->MaterialParams.Roughness = 0.f;
            PeriphTarget->MaterialParams.Anisotropy = 0.f;
        }
    }
}

void PeriphSystem::Tick(float DeltaTime, bool bIsReplaying, bool bInCleanRoomExperiment, 
const UCameraComponent *Camera, const AEgoSensor *EgoSensor)
{
    // if replaying, don't tick periph system
    if (bIsReplaying || World == nullptr)
    {
        // replay of legacy periph target is handled in UpdateData
        if (PeriphTarget != nullptr && PeriphTarget->IsActive())
            PeriphTarget->Deactivate();
        if (Cross != nullptr && Cross->IsActive())
            Cross->Deactivate();
        return;
    }

    const FVector &CameraLoc = Camera->GetComponentLocation();
    const FRotator &CameraRot = Camera->GetComponentRotation();
    if (bUseFixedCross)
    {
        if (bInCleanRoomExperiment)
        {   
            Cross->Activate();
            // TODO may not need this special case
            // init the first one 
            if (NumFCMoves == 0)
            {
                CrossVector = CameraRot.RotateVector(FVector::ForwardVector);
                FixCrossLoc = CameraLoc + CrossVector * TargetRenderDistance * 100.f;
                Cross->SetActorLocation(FixCrossLoc);
                Cross->SetActorRotation(CameraRot);
                
                // init the bounds for the FC Location
                HeadNeutralLoc = CameraLoc; HeadNeutralRot = CameraRot;
                NumFCMoves += 1;
            }
            
            // FC just got moved
            if (TimeSinceLastFCMove == 0.f)
            {
                // now that FC has moved, wait to do other stuff until gaze is close to the thing
                auto Gaze_dir_rel = EgoSensor->GetData()->GetGazeDir(DReyeVR::Gaze::COMBINED);
                auto Gaze_origin_abs = CameraLoc + EgoSensor->GetData()->GetGazeOrigin(DReyeVR::Gaze::COMBINED);
                auto Gaze_posn_rel = CameraRot.RotateVector(Gaze_dir_rel);
                auto Gaze_posn_abs = Gaze_origin_abs + Gaze_posn_rel;                
                Gaze_posn_rel.Normalize();
                
                auto FC_dir = FixCrossLoc - Gaze_origin_abs;
                UE_LOG(LogTemp, Log, TEXT("FC_dir:%s"), *FC_dir.ToString());
                FC_dir.Normalize();
                float DotProd = FVector::DotProduct(FC_dir, Gaze_posn_rel);
                float angleGaze2FC = UKismetMathLibrary::Acos(DotProd);
                
                UE_LOG(LogTemp, Log, TEXT("Angle between gaze and FC \t  %f"), angleGaze2FC);
                if ( UKismetMathLibrary::Abs(angleGaze2FC) > 0.2 ){ // 0.2rad = 11deg
                    // can't start the clock until gaze gets closer to FC
                    return;
                }                


                // update time for the next periph trigger -- this is basically the OFD
                PeriphBool = (FMath::RandRange(0.f, 1.f) <= PeriphSpawnRatio) ? true : false;
                NextPeriphTrigger = FMath::RandRange(TimeBetweenFlashFC.X, TimeBetweenFlashFC.Y) // make sure you get the float version of this fn
                +  FixationMoveTimeOffset;   // it takes some non zero time to start a new fixation 
                if (PeriphBool)
                {
                    UE_LOG(LogTemp, Log, TEXT("Next periph target gen \t @ %f"), NextPeriphTrigger);
                }
                else
                {
                    UE_LOG(LogTemp, Log, TEXT("No periph target gen"));
                }
                // generate random position for the next periph target
                const float RandYaw = FMath::RandRange(PeriphYawBounds.X, PeriphYawBounds.Y);
                const float RandPitch = FMath::RandRange(PeriphPitchBounds.X-FixCrossPitchExtension,
                    PeriphPitchBounds.Y+FixCrossPitchExtension);
                const float Roll = 0.f;
                PeriphRotator = FRotator(RandPitch, RandYaw, Roll);
            }
            // time to switch on PT
            else if (LastPeriphTick <= NextPeriphTrigger 
                        && TimeSinceLastFlash > NextPeriphTrigger
                    )
            {
                if (PeriphBool)
                {
                    // turn on periph target
                    PeriphTarget->Activate();
                    UE_LOG(LogTemp, Log, TEXT("Periph Target On \t @ %f"), UGameplayStatics::GetRealTimeSeconds(World));
                }
            }
            // time to switch off PT
            else if (LastPeriphTick <= NextPeriphTrigger + FlashDuration
                        && TimeSinceLastFlash > NextPeriphTrigger + FlashDuration 
                    )
            {
                if (PeriphBool)
                {
                    // turn off periph target
                    PeriphTarget->Deactivate();
                    UE_LOG(LogTemp, Log, TEXT("Periph Target On \t @ %f"), UGameplayStatics::GetRealTimeSeconds(World));
                    TimeSinceLastFlash = 0.f;
                }               

                NextFCMove = TimeSinceLastFCMove + DeltaTime
                    + FMath::RandRange(TimeFlashToMoveFC.X, TimeFlashToMoveFC.Y);
            }
            // time to move the FC
            else if (LastPeriphTick > NextPeriphTrigger + FlashDuration 
                    && TimeSinceLastFCMove > NextFCMove)
            {
                UE_LOG(LogTemp, Log, TEXT("Moving the FC \t @ %f"), UGameplayStatics::GetRealTimeSeconds(World));
                                                
                const float RandYaw = FMath::RandRange(FCYawBounds.X, FCYawBounds.Y);
                const float RandPitch = FMath::RandRange(FCPitchBounds.X, FCPitchBounds.Y);
                const float Roll = 0.f;
                CrossVector = FRotator(RandPitch, RandYaw, Roll).Vector();
                FixCrossLoc = HeadNeutralLoc + HeadNeutralRot.RotateVector(CrossVector) * TargetRenderDistance * 100.f;
                // FixCrossLoc = CameraLoc + CameraRot.RotateVector(CrossVector) * TargetRenderDistance * 100.f;                
                 // if no head movement compensation for FC, this is where loc is set
                Cross->SetActorLocation(FixCrossLoc);
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
                // auto Gaze2FCVec = FixCrossLoc - GazeDir;
                Cross->SetActorLocation(FixCrossLoc);                
            }

            // turn the FC so it always faces the participant 
            // const FRotator LookAtCamRot = (CameraLoc - FixCrossLoc).Rotation();
            const FRotator LookAtCamRot = UKismetMathLibrary::FindLookAtRotation(FixCrossLoc, CameraLoc);
            FixCrossRot = LookAtCamRot; // this is where the FC should be facing to face the first person cam always
            Cross->SetActorRotation(LookAtCamRot);


            if (PeriphTarget->IsActive())
            {
                const FVector PeriphFinal = -FixCrossRot.RotateVector((PeriphRotator).Vector());
                PeriphTarget->SetActorLocation(CameraLoc + PeriphFinal * TargetRenderDistance * 100.f);
            }

            
        }
        else if (Cross->IsActive())
        {
            Cross->Deactivate();
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
                PeriphTarget->Activate();
                UE_LOG(LogTemp, Log, TEXT("Periph Target On \t @ %f"), UGameplayStatics::GetRealTimeSeconds(World));
            }
            else if (LastPeriphTick <= NextPeriphTrigger + FlashDuration &&
                     TimeSinceLastFlash > NextPeriphTrigger + FlashDuration)
            {
                // turn off periph target
                PeriphTarget->Deactivate();
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
        if (PeriphTarget->IsActive())
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