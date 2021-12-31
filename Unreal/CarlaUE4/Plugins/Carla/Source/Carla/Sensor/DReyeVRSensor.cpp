#include "Carla/Sensor/DReyeVRSensor.h"
#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaStatics.h" // GetGameInstance

#include <sstream>
#include <string>

// vector types for serialization
#include "carla/geom/Vector2D.h"
#include "carla/geom/Vector3D.h"

class DReyeVR::SensorData *ADReyeVRSensor::Data = nullptr;

ADReyeVRSensor::ADReyeVRSensor(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    // no need for any other initialization
    PrimaryActorTick.bCanEverTick = true;
    ADReyeVRSensor::Data = new DReyeVR::SensorData();
}

FActorDefinition ADReyeVRSensor::GetSensorDefinition()
{
    // What our sensor is going to be called:
    auto Definition = UActorBlueprintFunctionLibrary::MakeGenericSensorDefinition(
        TEXT("DReyeVR"),        // folder directory   "dreyevr"
        TEXT("DReyeVRSensor")); // sensor name        "dreyevrsensor"

    /// NOTE: only has EActorAttributeType for bool, int, float, string, and RGBColor
    // see /Plugins/Carla/Source/Carla/Actor/ActorAttribute.h for the whole list

    // append all Variable variations to the definition
    // Definition.Variations.Append();

    return Definition;
}

void ADReyeVRSensor::Set(const FActorDescription &Description)
{
    Super::Set(Description);
}

void ADReyeVRSensor::SetOwner(AActor *Owner)
{
    check(Owner != nullptr);
    Super::SetOwner(Owner);
    // Set Transform to the same as the Owner
    SetActorLocation(Owner->GetActorLocation());
    SetActorRotation(Owner->GetActorRotation());
}

void ADReyeVRSensor::BeginPlay()
{
    Super::BeginPlay();
    World = GetWorld();

    UCarlaGameInstance *CarlaGame = UCarlaStatics::GetGameInstance(World);
    SetEpisode(*(CarlaGame->GetCarlaEpisode()));
    SetDataStream(CarlaGame->GetServer().OpenStream()); // initialize boost::optional<Stream>
}

void ADReyeVRSensor::BeginDestroy()
{
    Super::BeginDestroy();
}

void ADReyeVRSensor::PostPhysTick(UWorld *W, ELevelTick TickType, float DeltaSeconds)
{
    /// NOTE: this function defines the routine for streaming data to the PythonAPI
    if (!this->StreamSensorData) // param for enabling or disabling the data streaming
        return;
    auto Stream = GetDataStream(*this);

    struct // overloaded lambdas to convert UE4 types to carla::geom types
    {
        carla::geom::Vector3D operator()(const FVector &In)
        {
            return carla::geom::Vector3D{In.X, In.Y, In.Z};
        };
        carla::geom::Vector3D operator()(const FRotator &In)
        {
            return carla::geom::Vector3D{In.Pitch, In.Roll, In.Yaw};
        };
        carla::geom::Vector2D operator()(const FVector2D &In)
        {
            return carla::geom::Vector2D{In.X, In.Y};
        };
        carla::rpc::String operator()(const FString &In)
        {
            return carla::rpc::FromFString(In);
        };
    } ToGeom;

    /// TODO: refactor this somehow
    Stream.Send(*this,
                Data->GetTimestampDevice(),                          // Timestamp of SRanipal (ms)
                Data->GetTimestampCarla(),                           // Timestamp of Carla (ms)
                Data->GetFrameSequence(),                            // Frame sequence
                ToGeom(Data->GetGazeDir()),                          // Combined gaze ray direction
                ToGeom(Data->GetGazeOrigin()),                       // Stream EyeOrigin Vec3
                Data->GetGazeValidity(),                             // Validity of combined gaze
                Data->GetGazeVergence(),                             // Vergence (float) of combined ray
                ToGeom(Data->GetHMDLocation()),                      // HMD absolute location
                ToGeom(Data->GetHMDRotation()),                      // HMD absolute rotation
                ToGeom(Data->GetGazeDir(DReyeVR::Gaze::LEFT)),       // Left eye gaze ray direction
                ToGeom(Data->GetGazeOrigin(DReyeVR::Gaze::LEFT)),    // Left eye gaze origin
                Data->GetGazeValidity(DReyeVR::Gaze::LEFT),          // Validity of left gaze
                ToGeom(Data->GetGazeDir(DReyeVR::Gaze::RIGHT)),      // Right eye gaze ray direction
                ToGeom(Data->GetGazeOrigin(DReyeVR::Gaze::RIGHT)),   // Dight eye gaze origin
                Data->GetGazeValidity(DReyeVR::Gaze::RIGHT),         // Validity of right gaze
                Data->GetEyeOpenness(DReyeVR::Eye::LEFT),            // Left eye openness
                Data->GetEyeOpennessValidity(DReyeVR::Eye::LEFT),    // Validity of left eye openness
                Data->GetEyeOpenness(DReyeVR::Eye::RIGHT),           // Right eye openness
                Data->GetEyeOpennessValidity(DReyeVR::Eye::RIGHT),   // Validity of right eye openness
                ToGeom(Data->GetPupilPosition(DReyeVR::Eye::LEFT)),  // Left pupil position
                Data->GetPupilPositionValidity(DReyeVR::Eye::LEFT),  // Validity of left eye posn
                ToGeom(Data->GetPupilPosition(DReyeVR::Eye::RIGHT)), // Right pupil position
                Data->GetPupilPositionValidity(DReyeVR::Eye::RIGHT), // Validity of left eye posn
                Data->GetPupilDiameter(DReyeVR::Eye::LEFT),          // Left eye diameter (mm)
                Data->GetPupilDiameter(DReyeVR::Eye::RIGHT),         // Right eye diameter (mm)
                ToGeom(Data->GetFocusActorName()),                   // Focus Actor's name
                ToGeom(Data->GetFocusActorPoint()),                  // Focus Actor's location in world space
                Data->GetFocusActorDistance(),                       // Focus Actor's distance to the sensor
                Data->GetUserInputs().Throttle,                      // Vehicle input throttle
                Data->GetUserInputs().Steering,                      // Vehicle input steering
                Data->GetUserInputs().Brake,                         // Vehicle input brake
                Data->GetUserInputs().ToggledReverse,                // Vehicle input gear (reverse, fwd)
                Data->GetUserInputs().HoldHandbrake                  // Vehicle input handbrake
    );
}

/////////////////////////:DREYEVRSENSORDATA://////////////////////////////
/// NOTE: this to define the static variables that are set by the replayer when
//  replaying files to animate gaze ray data
bool ADReyeVRSensor::bIsReplaying = false; // not replaying initially
FTransform ADReyeVRSensor::EgoReplayTransform = FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(1, 1, 1));

void ADReyeVRSensor::UpdateReplayData(const DReyeVR::SensorData &RecorderData, const FTransform &EgoTrans,
                                      const double Per)
{
    // update global values
    ADReyeVRSensor::bIsReplaying = true;
    ADReyeVRSensor::EgoReplayTransform = EgoTrans;
    if (ADReyeVRSensor::Data != nullptr)
    {
        // update local values
        FVector NewCameraPose;
        FRotator NewCameraRot;
        InterpPositionAndRotation(ADReyeVRSensor::Data->GetHMDLocation(), // old location
                                  ADReyeVRSensor::Data->GetHMDRotation(), // old rotation
                                  RecorderData.GetHMDLocation(),          // new location
                                  RecorderData.GetHMDRotation(),          // new rotation
                                  Per, NewCameraPose, NewCameraRot);
        (*ADReyeVRSensor::Data) = RecorderData;
        // update camera positions to the interpolated ones
        ADReyeVRSensor::Data->UpdateCamera(NewCameraPose, NewCameraRot);
    }
}

// reposition actors
void ADReyeVRSensor::InterpPositionAndRotation(const FVector &Pos1, const FRotator &Rot1, const FVector &Pos2,
                                               const FRotator &Rot2, const double Per, FVector &Location,
                                               FRotator &Rotation)
{
    // inspired by CarlaReplayerHelper.cpp:CarlaReplayerHelper::ProcessReplayerPosition
    check(Per >= 0.f && Per <= 1.f);
    if (Per == 0.0) // check to assign first position or interpolate between both
    {
        // assign position 1 & ego posn shouldn't matter
        Location = Pos1;
        Rotation = Rot1;
    }
    else
    {
        // interpolate hmd location & rotation
        Location = FMath::Lerp(Pos1, Pos2, Per);
        Rotation = FMath::Lerp(Rot1, Rot2, Per);
    }
}
