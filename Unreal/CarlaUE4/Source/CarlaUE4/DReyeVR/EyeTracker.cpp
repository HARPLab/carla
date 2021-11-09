#include "EyeTracker.h"

#include "Kismet/KismetMathLibrary.h"   // Sin, Cos, Normalize
#include "UObject/UObjectBaseUtility.h" // GetName

#include <string>

#ifndef NO_DREYEVR_EXCEPTIONS
#include <exception>
#include <typeinfo>

namespace carla
{
void throw_exception(const std::exception &e)
{
    // It should never reach this part.
    std::terminate();
}
} // namespace carla
#endif

AEyeTracker::AEyeTracker()
{
    SensorData = new struct DReyeVR::SensorData;
    ReadDReyeVRConfig();
    ReadConfigValue("EyeTracker", "RecordFrames", bCaptureFrameData);
    ReadConfigValue("EyeTracker", "FrameWidth", FrameCapWidth);
    ReadConfigValue("EyeTracker", "FrameHeight", FrameCapHeight);
    ReadConfigValue("EyeTracker", "FrameDir", FrameCapLocation);
    ReadConfigValue("EyeTracker", "FrameName", FrameCapFilename);

    if (bCaptureFrameData)
    {
        // Frame capture
        CaptureRenderTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("CaptureRenderTarget_DReyeVR"));
        CaptureRenderTarget->CompressionSettings = TextureCompressionSettings::TC_Default;
        CaptureRenderTarget->SRGB = false;
        CaptureRenderTarget->bAutoGenerateMips = false;
        CaptureRenderTarget->bGPUSharedFlag = true;
        CaptureRenderTarget->ClearColor = FLinearColor::Black;
        CaptureRenderTarget->UpdateResourceImmediate(true);
        // CaptureRenderTarget->OverrideFormat = EPixelFormat::PF_FloatRGB;
        CaptureRenderTarget->AddressX = TextureAddress::TA_Clamp;
        CaptureRenderTarget->AddressY = TextureAddress::TA_Clamp;
        const bool bInForceLinearGamma = false;
        CaptureRenderTarget->InitCustomFormat(FrameCapWidth, FrameCapHeight, PF_B8G8R8A8, bInForceLinearGamma);
        check(CaptureRenderTarget->GetSurfaceWidth() > 0 && CaptureRenderTarget->GetSurfaceHeight() > 0);

        FrameCap = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("FrameCap"));
        FrameCap->SetupAttachment(FirstPersonCam);
        FrameCap->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
        FrameCap->bCaptureOnMovement = false;
        FrameCap->bCaptureEveryFrame = false;
        FrameCap->bAlwaysPersistRenderingState = true;
        FrameCap->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

        FrameCap->Deactivate();
        FrameCap->TextureTarget = CaptureRenderTarget;
        FrameCap->UpdateContent();
        FrameCap->Activate();
    }
    if (FrameCap && !bCaptureFrameData)
    {
        FrameCap->Deactivate();
    }
}

void AEyeTracker::BeginPlay()
{
    Super::BeginPlay();

    World = GetWorld();

#if USE_SRANIPAL
    UE_LOG(LogTemp, Warning, TEXT("Attempting to use SRanipal eye tracking"));
    // Initialize the SRanipal eye tracker (WINDOWS ONLY)
    SRanipalFramework = SRanipalEye_Framework::Instance();
    SRanipal = SRanipalEye_Core::Instance();
    // no easily discernible difference between v1 and v2
    /// TODO: use the status output from StartFramework to determine if SRanipal loaded successfully
    SRanipalFramework->StartFramework(SupportedEyeVersion::version1);
    // SRanipal->SetEyeParameter_() // can set the eye gaze jitter parameter
    // see SRanipal_Eyes_Enums.h
    // Get the reference timing to synchronize the SRanipal timer with Carla
    SRanipal->GetEyeData_(&EyeData);
    TimestampRef = EyeData.timestamp;
#else
    UE_LOG(LogTemp, Warning, TEXT("NOT using SRanipal eye tracking"));
#endif
    // Spawn the (cpp) DReyeVR Carla sensor and attach to self:
    FActorSpawnParameters SpawnInfo; // empty for now
    SpawnInfo.Owner = this;
    SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    cppDReyeVRSensor = World->SpawnActor<ADReyeVRSensor>(GetActorLocation(), GetActorRotation(), SpawnInfo);
    // Attach the DReyeVRSensor as a child to self
    cppDReyeVRSensor->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);

    // Set up frame capture
    if (bCaptureFrameData)
    {
        // create out dir
        /// TODO: add check for absolute paths
        FrameCapLocation = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + FrameCapLocation);
        UE_LOG(LogTemp, Log, TEXT("Outputting frame capture data to %s"), *FrameCapLocation);
        IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
        if (!PlatformFile.DirectoryExists(*FrameCapLocation))
            PlatformFile.CreateDirectory(*FrameCapLocation);
    }
}

void AEyeTracker::BeginDestroy()
{
#if USE_SRANIPAL
    if (SRanipalFramework)
    {
        SRanipalFramework->StopFramework();
        SRanipalEye_Framework::DestroyEyeFramework();
    }
    if (SRanipal)
        SRanipalEye_Core::DestroyEyeModule();
#endif
    Super::BeginDestroy();
}

void AEyeTracker::SetPlayer(APlayerController *P)
{
    Player = P;
}

void AEyeTracker::SetCamera(UCameraComponent *FPSCamInEgoVehicle)
{
    // Assign the FPS camera pointer
    FirstPersonCam = FPSCamInEgoVehicle;
}

void AEyeTracker::SetInputs(const DReyeVR::UserInputs &inputs)
{
    SensorData->Inputs = inputs;
}

void AEyeTracker::UpdateEgoVelocity(const float Velocity)
{
    EgoVelocity = Velocity;
}

void AEyeTracker::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!cppDReyeVRSensor->GetIsReplaying()) // only update the sensor with local values if not replaying
    {
        // ftime_s is used to get the UE4 (carla) timestamp of the world at this tick
        double ftime_s = UGameplayStatics::GetRealTimeSeconds(World);
#if USE_SRANIPAL
        /// NOTE: the GazeRay is the normalized direction vector of the actual gaze "ray"
        // Getting real eye tracker data
        check(SRanipal != nullptr);
        // Get the "EyeData" which holds useful information such as the timestamp
        SRanipal->GetEyeData_(&EyeData);
        SensorData->TimestampSR = EyeData.timestamp - TimestampRef;
        SensorData->FrameSequence = EyeData.frame_sequence;
        // shortcuts to eye datum
        auto Combined = &(SensorData->Combined);
        auto Left = &(SensorData->Left);
        auto Right = &(SensorData->Right);
        // Assigns EyeOrigin and Gaze direction (normalized) of combined gaze
        Combined->GazeValid = SRanipal->GetGazeRay(GazeIndex::COMBINE, Combined->Origin, Combined->GazeRay);
        // Assign Left/Right Gaze direction
        /// NOTE: the eye gazes are reversed at the lowest level bc SRanipal has a bug in their
        // libraries that flips these when collected from the sensor. We can verify this by
        // plotting debug lines for the left and right eye gazes and notice that if we close
        // one eye, the OTHER eye's gaze is null, when it should be the closed eye instead...
        // see: https://forum.vive.com/topic/9306-possible-bug-in-unreal-sdk-for-leftright-eye-gazes
        if (SRANIPAL_EYE_SWAP_FIXED) // if the latest SRanipal does not have this bug
        {
            Left->GazeValid = SRanipal->GetGazeRay(GazeIndex::LEFT, Left->Origin, Left->GazeRay);
            Right->GazeValid = SRanipal->GetGazeRay(GazeIndex::RIGHT, Right->Origin, Right->GazeRay);
        }
        else // this is the default case which we were dealing with during development
        {
            Left->GazeValid = SRanipal->GetGazeRay(GazeIndex::LEFT, Left->Origin, Right->GazeRay);
            Right->GazeValid = SRanipal->GetGazeRay(GazeIndex::RIGHT, Right->Origin, Left->GazeRay);
        }
        // Assign Eye openness
        Left->EyeOpenValid = SRanipal->GetEyeOpenness(EyeIndex::LEFT, Left->EyeOpenness);
        Right->EyeOpenValid = SRanipal->GetEyeOpenness(EyeIndex::RIGHT, Right->EyeOpenness);
        // Assign Pupil positions
        Left->PupilPosValid = SRanipal->GetPupilPosition(EyeIndex::LEFT, Left->PupilPos);
        Right->PupilPosValid = SRanipal->GetPupilPosition(EyeIndex::RIGHT, Right->PupilPos);
        // Assign Pupil Diameters
        Left->PupilDiam = EyeData.verbose_data.left.pupil_diameter_mm;
        Right->PupilDiam = EyeData.verbose_data.right.pupil_diameter_mm;
        // Assign FFocus information
        /// NOTE: the ECC_GameTraceChannel4 line trace allows the trace to ignore the vehicle
        // you can see ECC_GameTraceChannel4 in DefaultEngine.ini as one of the 18 custom channels
        const float radius = 0.f; // 0 for a point, >0 for a sphear trace
        SRanipalFocus(ECC_GameTraceChannel4, FocusInfo, radius);
        if (FocusInfo.actor != nullptr)
            FocusInfo.actor->GetName(SensorData->FocusActorName);
        else
            SensorData->FocusActorName = FString("None"); // empty string, not looking at any actor
        SensorData->FocusActorPoint = FocusInfo.point;
        SensorData->FocusActorDist = FocusInfo.distance;
        // UE_LOG(LogTemp, Log, TEXT("Focus Actor: %s"), *SensorData->FocusActorName);
#else
        SensorData->TimestampSR++; // iterate the fake "SRanipal" timestamp
        // Generate dummy values for Gaze Ray based off time, goes in circles in front of the user
        auto Combined = &(SensorData->Combined);
        Combined->GazeRay.X = 5.0;
        Combined->GazeRay.Y = UKismetMathLibrary::Cos(ftime_s);
        Combined->GazeRay.Z = UKismetMathLibrary::Sin(ftime_s);
        UKismetMathLibrary::Vector_Normalize(Combined->GazeRay, 0.0001);

        // Assign the origin position to the (3D space) origin
        Combined->GazeValid = true; // for our Linux case, this is valid
                                    // not going to assign anything for the L/R eye tracker fields

        // Assign the endpoint of the combined position (faked in Linux) to the left & right gazes too
        SensorData->Left.GazeRay = Combined->GazeRay;
        SensorData->Right.GazeRay = Combined->GazeRay;

#endif
        // Update the ego velocity
        SensorData->Velocity = EgoVelocity;
        // Update the Carla tick timestamp
        SensorData->TimestampCarla = int64_t(ftime_s * 1000);
        if (FirstPersonCam != nullptr)
        {
            // Update the hmd location
            SensorData->HMDLocation = FirstPersonCam->GetRelativeLocation();
            SensorData->HMDRotation = FirstPersonCam->GetRelativeRotation();
        }
        // The Vergence will be calculated with SRanipal if available, else just 1.0f
        Combined->Vergence = CalculateVergenceFromDirections();
        // Update the DReyeVR Carla sensor with the most current values
        /// NOTE: both the cpp and pyDReyeVRSensor share the same static data, so we only need to update one
        cppDReyeVRSensor->Update(SensorData);
    }
    else
    {
        // this gets reached when the simulator is replaying data from a carla log
        // update the local SensorData with the global (replayer made) SensorData
        SensorData = cppDReyeVRSensor->Snapshot;
        FirstPersonCam->SetRelativeRotation(SensorData->HMDRotation, false, nullptr, ETeleportType::None);
        FirstPersonCam->SetRelativeLocation(SensorData->HMDLocation, false, nullptr, ETeleportType::None);
    }
    // frame capture
    if (bCaptureFrameData && FrameCap && FirstPersonCam)
    {
        FMinimalViewInfo DesiredView;
        FirstPersonCam->GetCameraView(0, DesiredView);
        FrameCap->SetCameraView(DesiredView); // move camera to the FirstPersonCam view
        FrameCap->CaptureScene();             // also available: CaptureSceneDeferred()
        const FString Suffix = FString::Printf(TEXT("%04d.png"), TickCount);
        SaveFrameToDisk(*CaptureRenderTarget, FPaths::Combine(FrameCapLocation, FrameCapFilename + Suffix));
    }
    TickCount++;
}

/// ========================================== ///
/// ---------------:GETTERS:------------------ ///
/// ========================================== ///
FVector AEyeTracker::GetCenterGazeRay() const
{
    return SensorData->Combined.GazeRay;
}

FVector AEyeTracker::GetCenterOrigin() const
{
    return SensorData->Combined.Origin;
}

float AEyeTracker::GetVergence() const
{
    return SensorData->Combined.Vergence;
}

FVector AEyeTracker::GetLeftGazeRay() const
{
    return SensorData->Left.GazeRay;
}

FVector AEyeTracker::GetLeftOrigin() const
{
    return SensorData->Left.Origin;
}

FVector AEyeTracker::GetRightGazeRay() const
{
    return SensorData->Right.GazeRay;
}

FVector AEyeTracker::GetRightOrigin() const
{
    return SensorData->Right.Origin;
}

/// ========================================== ///
/// ---------------:HELPERS:------------------ ///
/// ========================================== ///

// Find an existing (already spawned) DReyeVR sensor instance in the world if one exists
// This makes it very straightforward for a Python client to spawn and store the client-side sensor, which gets
// updated from the server's (ego-vehicle's) POV
bool AEyeTracker::FindPyDReyeVRSensor()
{
    // not efficient to call this function on every tick(), therefore we'll call it on a *record* trigger
    // returns whether or not a DReyeVR sensor has been spawned and reset
    TArray<AActor *> FoundDReyeVRSensors;
    UGameplayStatics::GetAllActorsOfClass(World, ADReyeVRSensor::StaticClass(), FoundDReyeVRSensors);
    // ensure only one is spawned else we are confused
    if (FoundDReyeVRSensors.Num() <= 1) // first ADReyeVRSensor is the cppDReyeVRSensor
    {
        UE_LOG(LogTemp, Log, TEXT("Unable to find any DReyeVR sensors spawned in World!"));
        return false;
    }
    else
    {
        /// NOTE: there will always be >=ONE ADReyeVRSensors because the cppDReyeVRSensor is spawned from this class
        // so we are looking for >1
        if (FoundDReyeVRSensors.Num() > 2)
            UE_LOG(LogTemp, Log, TEXT("Found multiple DReyeVR sensors in the World, defaulting to the last one"));
        const int lastIdx = FoundDReyeVRSensors.Num() - 1;
        pyDReyeVRSensor = CastChecked<ADReyeVRSensor>(FoundDReyeVRSensors[lastIdx]); // always default to the first one
        pyDReyeVRSensor->ClientInitialized = true; // this instance is allowed to stream
    }
    if (!pyDReyeVRSensor)
    {
        UE_LOG(LogTemp, Log, TEXT("Did not assign a DReyeVR Sensor to the EyeTracker"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Assigned a DReyeVR Sensor in the EyeTracker"));
    }

    return true;
}

bool AEyeTracker::ResetPyDReyeVRSensor()
{
    // reset the DReyeVRSensor
    // returns whether or not a DReyeVR sensor has been spawned and reset
    if (pyDReyeVRSensor != nullptr)
    {
        pyDReyeVRSensor = nullptr;
        UE_LOG(LogTemp, Log, TEXT("Reset the DReyeVR Sensor to a nullptr"));
        return true;
    }
    else
        UE_LOG(LogTemp, Log, TEXT("DReyeVR Sensor already reset to nullptr"));
    return false;
}

// This function expects a Windows machine (SRanipal)
#if USE_SRANIPAL
bool AEyeTracker::SRanipalFocus(const ECollisionChannel TraceChannel, FFocusInfo &F, const float radius = 0.f)
{
    if (Player == nullptr) // required for line trace
        return false;
    bool hit = false;
    const float maxDist = 100.f * 100.f; // 100m

    const FRotator WorldRot = SensorData->HMDRotation;
    const FVector WorldPos = SensorData->HMDLocation;
    const FVector GazeOrigin = WorldRot.RotateVector(SensorData->Combined.Origin) + WorldPos;
    const FVector GazeTarget = WorldRot.RotateVector(maxDist * SensorData->Combined.GazeRay);

    // Create collision information container.
    FCollisionQueryParams traceParam;
    traceParam = FCollisionQueryParams(FName("traceParam"), true, Player->PlayerCameraManager);
    traceParam.bTraceComplex = true;
    traceParam.bReturnPhysicalMaterial = false;
    FHitResult hitResult;
    hitResult = FHitResult(ForceInit);

    // Single line trace
    if (radius == 0.f)
    {
        hit = World->LineTraceSingleByChannel(hitResult, GazeOrigin, GazeTarget, TraceChannel, traceParam);
    }
    // Sphear line trace
    else
    {
        FCollisionShape sphear = FCollisionShape();
        sphear.SetSphere(radius);
        hit = World->SweepSingleByChannel(hitResult, GazeOrigin, GazeTarget, FQuat(0.f, 0.f, 0.f, 0.f), TraceChannel,
                                          sphear, traceParam);
    }
    // Update fields
    F.actor = hitResult.Actor;
    F.distance = hitResult.Distance;
    F.point = hitResult.Location;
    F.normal = hitResult.Normal;
    return hit;
}
#endif

// Calculate the distance from the origin (between both eyes) to the focus point
/// NOTE: requires SRanipal (and therefore Windows)
float AEyeTracker::CalculateVergenceFromDirections() const
{
#if USE_SRANIPAL
    // Compute intersection of the rays in 3D space to compute distance to that point

    // Recall that a 'line' can be defined here as (L = origin(0) + t * direction(Dir)) for some t
    FVector L0 = GetLeftOrigin();
    FVector LDir = GetLeftGazeRay();
    FVector R0 = GetRightOrigin();
    FVector RDir = GetRightGazeRay();

    // Calculating shortest line segment intersecting both lines
    // Implementation sourced from http://paulbourke.net/geometry/pointlineplane/
    FVector L0R0 = L0 - R0;         // segment between L origin and R origin
    const double epsilon = 0.00001; // small positive real number

    // Calculating dot-product equation to find perpendicular shortest-line-segment
    double d1343 = L0R0.X * RDir.X + L0R0.Y * RDir.Y + L0R0.Z * RDir.Z;
    double d4321 = RDir.X * LDir.X + RDir.Y * LDir.Y + RDir.Z * LDir.Z;
    double d1321 = L0R0.X * LDir.X + L0R0.Y * LDir.Y + L0R0.Z * LDir.Z;
    double d4343 = RDir.X * RDir.X + RDir.Y * RDir.Y + RDir.Z * RDir.Z;
    double d2121 = LDir.X * LDir.X + LDir.Y * LDir.Y + LDir.Z * LDir.Z;
    double denom = d2121 * d4343 - d4321 * d4321;
    if (abs(denom) < epsilon)
        return 1.0f; // no intersection, would cause div by 0 err (potentially)
    double numer = d1343 * d4321 - d1321 * d4343;

    // calculate scalars (mu) that scale the unit direction XDir to reach the desired points
    double muL = numer / denom;                   // variable scale of direction vector for LEFT ray
    double muR = (d1343 + d4321 * (muL)) / d4343; // variable scale of direction vector for RIGHT ray

    // calculate the points on the respective rays that create the intersecting line
    FVector ptL = L0 + muL * LDir; // the point on the Left ray
    FVector ptR = R0 + muR * RDir; // the point on the Right ray

    FVector ShortestLineSeg = ptL - ptR; // the shortest line segment between the two rays
    // calculate the vector between the middle of the two endpoints and return its magnitude
    FVector ptM = (ptL + ptR) / 2.0; // middle point between two endpoints of shortest-line-segment
    FVector oM = (L0 + R0) / 2.0;    // midpoint between two (L & R) origins
    FVector FinalRay = ptM - oM;     // Combined ray between midpoints of endpoints
    // Note that at this point everything has been computed in terms of cm
    return FinalRay.Size() / 100.0f; // returns the magnitude of the vector (length in m)
#else
    // Not calculating vergence without real values
    return 1.0f;
#endif
}