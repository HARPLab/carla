#include "EgoSensor.h"

#include "Carla/Game/CarlaStatics.h"    // GetEpisode
#include "DReyeVRUtils.h"               // ReadConfigValue
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

AEgoSensor::AEgoSensor(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    ReadConfigValue("EgoSensor", "StreamSensorData", bStreamData);
    ReadConfigValue("EgoSensor", "RecordFrames", bCaptureFrameData);
    ReadConfigValue("EgoSensor", "FrameWidth", FrameCapWidth);
    ReadConfigValue("EgoSensor", "FrameHeight", FrameCapHeight);
    ReadConfigValue("EgoSensor", "FrameDir", FrameCapLocation);
    ReadConfigValue("EgoSensor", "FrameName", FrameCapFilename);

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
        FrameCap->SetupAttachment(Camera);
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

void AEgoSensor::BeginPlay()
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
    SRanipal->GetEyeData_(EyeData);
    TimestampRef = EyeData->timestamp;
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

    // Register EgoSensor with ActorRegistry
    FActorView::IdType ID = 513;
    FActorDescription SensorDescr;
    SensorDescr.Id = "sensor.dreyevr.dreyevrsensor";
    FString Tags = "EgoSensor,DReyeVR";
    UCarlaStatics::GetCurrentEpisode(World)->RegisterActor(*this, SensorDescr, Tags, ID);

    UE_LOG(LogTemp, Log, TEXT("Initialized DReyeVR Eye Tracker"));
}

void AEgoSensor::BeginDestroy()
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

void AEgoSensor::SetEgoVehicle(class AEgoVehicle *NewEgoVehicle)
{
    Vehicle = NewEgoVehicle;
    Camera = Vehicle->GetCamera();
    check(Vehicle);
}

void AEgoSensor::PrePhysTick(float DeltaSeconds)
{
    if (!bIsReplaying) // only update the sensor with local values if not replaying
    {
        const float Timestamp = int64_t(1000.f * UGameplayStatics::GetRealTimeSeconds(World));
        TickEyeTracker();                             // query the eye-tracker hardware for current data
        ComputeTraceFocusInfo(ECC_GameTraceChannel4); // compute gaze focus data
        ComputeEgoVars();                             // get all necessary ego-vehicle data

        // Update the internal sensor data that gets handed off to Carla (for recording/replaying/PythonAPI)
        GetData()->Update(Timestamp,                  // TimestampCarla (ms)
                          EyeSensorData,              // EyeTrackerData
                          EgoVars,                    // EgoVehicleVariables
                          FocusInfoData,              // FocusData
                          Vehicle->GetVehicleInputs() // User inputs
        );
    }
    // frame capture
    if (bCaptureFrameData && FrameCap && Camera)
    {
        FMinimalViewInfo DesiredView;
        Camera->GetCameraView(0, DesiredView);
        FrameCap->SetCameraView(DesiredView); // move camera to the Camera view
        FrameCap->CaptureScene();             // also available: CaptureSceneDeferred()
        const FString Suffix = FString::Printf(TEXT("%04d.png"), TickCount);
        SaveFrameToDisk(*CaptureRenderTarget, FPaths::Combine(FrameCapLocation, FrameCapFilename + Suffix));
    }
    TickCount++;
}

void AEgoSensor::TickEyeTracker()
{
    auto Combined = &(EyeSensorData.Combined);
    auto Left = &(EyeSensorData.Left);
    auto Right = &(EyeSensorData.Right);
#if USE_SRANIPAL
    /// NOTE: the GazeRay is the normalized direction vector of the actual gaze "ray"
    // Getting real eye tracker data
    check(SRanipal != nullptr);
    // Get the "EyeData" which holds useful information such as the timestamp
    SRanipal->GetEyeData_(EyeData);
    EyeSensorData.TimestampDevice = EyeData->timestamp - TimestampRef;
    EyeSensorData.FrameSequence = EyeData->frame_sequence;
    // Assigns EyeOrigin and Gaze direction (normalized) of combined gaze
    Combined->GazeValid = SRanipal->GetGazeRay(GazeIndex::COMBINE, Combined->GazeOrigin, Combined->GazeDir);
    // Assign Left/Right Gaze direction
    /// NOTE: the eye gazes are reversed at the lowest level bc SRanipal has a bug in their
    // libraries that flips these when collected from the sensor. We can verify this by
    // plotting debug lines for the left and right eye gazes and notice that if we close
    // one eye, the OTHER eye's gaze is null, when it should be the closed eye instead...
    // see: https://forum.vive.com/topic/9306-possible-bug-in-unreal-sdk-for-leftright-eye-gazes
    if (SRANIPAL_EYE_SWAP_FIXED) // if the latest SRanipal does not have this bug
    {
        Left->GazeValid = SRanipal->GetGazeRay(GazeIndex::LEFT, Left->GazeOrigin, Left->GazeDir);
        Right->GazeValid = SRanipal->GetGazeRay(GazeIndex::RIGHT, Right->GazeOrigin, Right->GazeDir);
    }
    else // this is the default case which we were dealing with during development
    {
        Left->GazeValid = SRanipal->GetGazeRay(GazeIndex::LEFT, Left->GazeOrigin, Right->GazeDir);
        Right->GazeValid = SRanipal->GetGazeRay(GazeIndex::RIGHT, Right->GazeOrigin, Left->GazeDir);
    }
    // Assign Eye openness
    Left->EyeOpennessValid = SRanipal->GetEyeOpenness(EyeIndex::LEFT, Left->EyeOpenness);
    Right->EyeOpennessValid = SRanipal->GetEyeOpenness(EyeIndex::RIGHT, Right->EyeOpenness);
    // Assign Pupil positions
    Left->PupilPositionValid = SRanipal->GetPupilPosition(EyeIndex::LEFT, Left->PupilPosition);
    Right->PupilPositionValid = SRanipal->GetPupilPosition(EyeIndex::RIGHT, Right->PupilPosition);
    // Assign Pupil Diameters
    Left->PupilDiameter = EyeData->verbose_data.left.pupil_diameter_mm;
    Right->PupilDiameter = EyeData->verbose_data.right.pupil_diameter_mm;
#else
    // Generate dummy values for Gaze Ray based off time, goes in circles in front of the user
    Combined->GazeDir.X = 5.0;
    // std::chrono::duration<double> Time = std::chrono::system_clock::now();
    const auto DeltaT = std::chrono::system_clock::now() - StartTime; // time difference since begin play
    const float TimeNow = std::chrono::duration_cast<std::chrono::milliseconds>(DeltaT).count() / 1000.f;
    Combined->GazeDir.Y = UKismetMathLibrary::Cos(TimeNow);
    Combined->GazeDir.Z = UKismetMathLibrary::Sin(TimeNow);
    UKismetMathLibrary::Vector_Normalize(Combined->GazeDir, 0.0001);

    // Assign the origin position to the (3D space) origin
    Combined->GazeValid = true; // for our Linux case, this is valid
    Combined->GazeOrigin = FVector::ZeroVector;

    // Assign the endpoint of the combined position (faked in Linux) to the left & right gazes too
    Left->GazeDir = Combined->GazeDir;
    Left->GazeOrigin = Combined->GazeOrigin + FVector(0, -5, 0);
    Right->GazeDir = Combined->GazeDir;
    Right->GazeOrigin = Combined->GazeOrigin + FVector(0, +5, 0);
#endif
    Combined->Vergence = ComputeVergence(Left->GazeOrigin, Left->GazeDir, Right->GazeOrigin, Right->GazeDir);
    // FPlatformProcess::Sleep(0.00833f); // use in async thread to get 120hz
}

void AEgoSensor::ComputeTraceFocusInfo(const ECollisionChannel TraceChannel, float TraceRadius)
{
    const float TraceLen = 100.f * 100.f; // 100m in world space
    const FRotator &WorldRot = GetData()->GetCameraRotation();
    const FVector &WorldPos = GetData()->GetCameraLocation();
    const FVector GazeOrigin = WorldRot.RotateVector(GetData()->GetGazeOrigin()) + WorldPos;
    const FVector GazeDir = WorldRot.RotateVector(TraceLen * GetData()->GetGazeDir());

    // Create collision information container.
    FCollisionQueryParams TraceParam;
    TraceParam = FCollisionQueryParams(FName("TraceParam"), true);
    TraceParam.AddIgnoredActor(Vehicle); // don't collide with the vehicle since that would be useless
    TraceParam.bTraceComplex = true;
    TraceParam.bReturnPhysicalMaterial = false;
    FHitResult Hit(EForceInit::ForceInit);
    bool bDidHit = false;

    // 0 for a point, >0 for a sphear trace
    TraceRadius = FMath::Max(TraceRadius, 0.f); // clamp to be positive

    if (TraceRadius == 0.f) // Single ray/line trace
    {
        bDidHit = World->LineTraceSingleByChannel(Hit, GazeOrigin, GazeDir, TraceChannel, TraceParam);
    }
    else // Sphear line trace
    {
        FCollisionShape Sphear = FCollisionShape();
        Sphear.SetSphere(TraceRadius);
        bDidHit = World->SweepSingleByChannel(Hit, GazeOrigin, GazeDir, FQuat(0.f, 0.f, 0.f, 0.f), TraceChannel, Sphear,
                                              TraceParam);
    }
    // Update fields
    FString ActorName = "None";
    if (Hit.Actor != nullptr)
        Hit.Actor->GetName(ActorName);
    // update internal data structure
    FocusInfoData = {Hit.Actor,
                     bDidHit,
                     Hit.Distance,
                     Hit.Location,              // world location of hit point
                     Hit.Location - GazeOrigin, // relative location of hit point
                     Hit.Normal,
                     ActorName};
}

void AEgoSensor::ComputeEgoVars()
{
    EgoVars.VehicleLocation = Vehicle->GetActorLocation();
    EgoVars.VehicleRotation = Vehicle->GetActorRotation();
    EgoVars.CameraLocation = Camera->GetRelativeLocation();
    EgoVars.CameraRotation = Camera->GetRelativeRotation();
    EgoVars.Velocity = Vehicle->GetVehicleForwardSpeed();
}

float AEgoSensor::ComputeVergence(const FVector &L0, const FVector &LDir, const FVector &R0, const FVector &RDir) const
{
    // Compute length of ray-to- intersection of the left and right eye gazes in 3D space (length in centimeters)
    // Recall that a 'line' can be defined as (L = origin(0) + t * direction(Dir)) for some t

    // Calculating shortest line segment intersecting both lines
    // Implementation sourced from http://paulbourke.net/geometry/pointlineplane/
    FVector L0R0 = L0 - R0; // segment between L origin and R origin
    if (L0R0.Size() == 0.f) // same origin
        return 0.f;
    const float epsilon = 0.00001f; // small positive real number

    // Calculating dot-product equation to find perpendicular shortest-line-segment
    float d1343 = L0R0.X * RDir.X + L0R0.Y * RDir.Y + L0R0.Z * RDir.Z;
    float d4321 = RDir.X * LDir.X + RDir.Y * LDir.Y + RDir.Z * LDir.Z;
    float d1321 = L0R0.X * LDir.X + L0R0.Y * LDir.Y + L0R0.Z * LDir.Z;
    float d4343 = RDir.X * RDir.X + RDir.Y * RDir.Y + RDir.Z * RDir.Z;
    float d2121 = LDir.X * LDir.X + LDir.Y * LDir.Y + LDir.Z * LDir.Z;
    float denom = d2121 * d4343 - d4321 * d4321;
    if (abs(denom) < epsilon)
        return 100.f; // no intersection, would cause div by 0 err (potentially)
    float numer = d1343 * d4321 - d1321 * d4343;

    // calculate scalars (mu) that scale the unit direction XDir to reach the desired points
    float muL = numer / denom;                   // variable scale of direction vector for LEFT ray
    float muR = (d1343 + d4321 * (muL)) / d4343; // variable scale of direction vector for RIGHT ray

    // calculate the points on the respective rays that create the intersecting line
    FVector ptL = L0 + muL * LDir; // the point on the Left ray
    FVector ptR = R0 + muR * RDir; // the point on the Right ray

    FVector ShortestLineSeg = ptL - ptR; // the shortest line segment between the two rays
    // calculate the vector between the middle of the two endpoints and return its magnitude
    FVector ptM = (ptL + ptR) / 2.0f; // middle point between two endpoints of shortest-line-segment
    FVector oM = (L0 + R0) / 2.0f;    // midpoint between two (L & R) origins
    FVector FinalRay = ptM - oM;      // Combined ray between midpoints of endpoints
    return FinalRay.Size();           // returns the magnitude of the vector (in cm)
}