#include "EyeTracker.h"

#include "Carla/Game/CarlaStatics.h"    // GetEpisode
#include "Kismet/KismetMathLibrary.h"   // Sin, Cos, Normalize
#include "UObject/UObjectBaseUtility.h" // GetName

#ifdef _WIN32
#include <windows.h> // required for file IO in Windows
#endif

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

AEyeTracker::AEyeTracker(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
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
    StartTime = std::chrono::system_clock::now();

#if USE_SRANIPAL
    // initialize SRanipal framework for eye tracking
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
    // Set up frame capture
    if (bCaptureFrameData)
    {
        // create out dir
        /// TODO: add check for absolute paths
        FrameCapLocation = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + FrameCapLocation);
        UE_LOG(LogTemp, Log, TEXT("Outputting frame capture data to %s"), *FrameCapLocation);
        IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
        if (!PlatformFile.DirectoryExists(*FrameCapLocation))
        {
#ifndef _WIN32
            // this only seems to work on Unix systems, else CreateDirectoryW is not linked
            PlatformFile.CreateDirectory(*FrameCapLocation);
#else
            // using Windows system calls
            CreateDirectory(*FrameCapLocation, NULL);
#endif
        }
    }

    // Register EyeTracker with ActorRegistry
    FActorView::IdType ID = 513;
    FActorDescription SensorDescr;
    SensorDescr.Id = "sensor.dreyevr.dreyevrsensor";
    UCarlaStatics::GetCurrentEpisode(World)->RegisterActor(*this, SensorDescr, ID);

    UE_LOG(LogTemp, Log, TEXT("Initialized DReyeVR Eye Tracker"));
}

void AEyeTracker::BeginDestroy()
{
    Super::BeginDestroy();
#if USE_SRANIPAL
    if (SRanipalFramework)
    {
        SRanipalFramework->StopFramework();
        SRanipalEye_Framework::DestroyEyeFramework();
    }
    if (SRanipal)
        SRanipalEye_Core::DestroyEyeModule();
#endif
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
    GetData()->Inputs = inputs;
}

void AEyeTracker::UpdateEgoVelocity(const float Velocity)
{
    EgoVelocity = Velocity;
}

void AEyeTracker::PrePhysTick(float DeltaSeconds)
{
    if (!this->GetIsReplaying()) // only update the sensor with local values if not replaying
    {
        // ftime_s is used to get the UE4 (carla) timestamp of the world at this tick
        double ftime_s = UGameplayStatics::GetRealTimeSeconds(World);
        // Get data from the hardware sensor
        GetData()->UpdateEyeTrackerData(this->TickSensor());

        // Assign FFocus information
        /// NOTE: the ECC_GameTraceChannel4 line trace allows the trace to ignore the vehicle
        // you can see ECC_GameTraceChannel4 in DefaultEngine.ini as one of the 18 custom channels
        const float TraceRadius = 0.f; // 0 for a point, >0 for a sphear trace
        DReyeVR::FocusInfo F;
        ComputeTraceFocusInfo(ECC_GameTraceChannel4, F, TraceRadius);
        if (F.Actor != nullptr)
            F.Actor->GetName(GetData()->FocusActorName);
        else
            GetData()->FocusActorName = FString("None"); // empty string, not looking at any actor
        GetData()->FocusActorPoint = F.Point;
        GetData()->FocusActorDist = F.Distance;
        // UE_LOG(LogTemp, Log, TEXT("Focus Actor: %s"), *Data->FocusActorName);

        // Update the ego velocity
        GetData()->Velocity = EgoVelocity;
        // Update the Carla tick timestamp
        GetData()->TimestampCarla = int64_t(ftime_s * 1000);
        if (FirstPersonCam != nullptr)
        {
            // Update the hmd location
            GetData()->HMDLocation = FirstPersonCam->GetRelativeLocation();
            GetData()->HMDRotation = FirstPersonCam->GetRelativeRotation();
        }
        // The Vergence will be calculated with SRanipal if available, else just 1.0f
        GetData()->Combined.Vergence = CalculateVergenceFromDirections();
        // Update the DReyeVR Carla sensor with the most current values
    }
    else
    {
        // this gets reached when the simulator is replaying data from a carla log
        // assign first person camera orientation and location
        FirstPersonCam->SetRelativeRotation(GetData()->HMDRotation, false, nullptr, ETeleportType::None);
        FirstPersonCam->SetRelativeLocation(GetData()->HMDLocation, false, nullptr, ETeleportType::None);
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

DReyeVR::SRanipalData AEyeTracker::TickSensor()
{
    DReyeVR::SRanipalData Data;
    auto Combined = &(Data.Combined);
    auto Left = &(Data.Left);
    auto Right = &(Data.Right);
#if USE_SRANIPAL
    /// NOTE: the GazeRay is the normalized direction vector of the actual gaze "ray"
    // Getting real eye tracker data
    check(SRanipal != nullptr);
    // Get the "EyeData" which holds useful information such as the timestamp
    SRanipal->GetEyeData_(EyeData);
    Data.TimestampSR = EyeData->timestamp - TimestampRef;
    Data.FrameSequence = EyeData->frame_sequence;
    // shortcuts to eye datum
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
    Left->PupilDiam = EyeData->verbose_data.left.pupil_diameter_mm;
    Right->PupilDiam = EyeData->verbose_data.right.pupil_diameter_mm;
#else
    // Generate dummy values for Gaze Ray based off time, goes in circles in front of the user
    Combined->GazeRay.X = 5.0;
    // std::chrono::duration<double> Time = std::chrono::system_clock::now();
    const auto DeltaT = std::chrono::system_clock::now() - StartTime; // time difference since begin play
    const float TimeNow = std::chrono::duration_cast<std::chrono::milliseconds>(DeltaT).count() / 1000.f;
    Combined->GazeRay.Y = UKismetMathLibrary::Cos(TimeNow);
    Combined->GazeRay.Z = UKismetMathLibrary::Sin(TimeNow);
    UKismetMathLibrary::Vector_Normalize(Combined->GazeRay, 0.0001);

    // Assign the origin position to the (3D space) origin
    Combined->GazeValid = true; // for our Linux case, this is valid
                                // not going to assign anything for the L/R eye tracker fields

    // Assign the endpoint of the combined position (faked in Linux) to the left & right gazes too
    Left->GazeRay = Combined->GazeRay;
    Right->GazeRay = Combined->GazeRay;
#endif
    // FPlatformProcess::Sleep(0.00833f); // use in async thread to get 120hz
    return Data;
}

/// ========================================== ///
/// ---------------:GETTERS:------------------ ///
/// ========================================== ///
FVector AEyeTracker::GetCenterGazeRay() const
{
    return GetData()->Combined.GazeRay;
}

FVector AEyeTracker::GetCenterOrigin() const
{
    return GetData()->Combined.Origin;
}

float AEyeTracker::GetVergence() const
{
    return GetData()->Combined.Vergence;
}

FVector AEyeTracker::GetLeftGazeRay() const
{
    return GetData()->Left.GazeRay;
}

FVector AEyeTracker::GetLeftOrigin() const
{
    return GetData()->Left.Origin;
}

FVector AEyeTracker::GetRightGazeRay() const
{
    return GetData()->Right.GazeRay;
}

FVector AEyeTracker::GetRightOrigin() const
{
    return GetData()->Right.Origin;
}

/// ========================================== ///
/// ---------------:HELPERS:------------------ ///
/// ========================================== ///

bool AEyeTracker::ComputeTraceFocusInfo(const ECollisionChannel TraceChannel, DReyeVR::FocusInfo &F,
                                        const float radius = 0.f)
{
    if (Player == nullptr) // required for line trace
        return false;
    bool hit = false;
    const float maxDist = 100.f * 100.f; // 100m

    const FRotator WorldRot = GetData()->HMDRotation;
    const FVector WorldPos = GetData()->HMDLocation;
    const FVector GazeOrigin = WorldRot.RotateVector(GetData()->Combined.Origin) + WorldPos;
    const FVector GazeTarget = WorldRot.RotateVector(maxDist * GetData()->Combined.GazeRay);

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
    F.Actor = hitResult.Actor;
    F.Distance = hitResult.Distance;
    F.Point = hitResult.Location;
    F.Normal = hitResult.Normal;
    return hit;
}

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