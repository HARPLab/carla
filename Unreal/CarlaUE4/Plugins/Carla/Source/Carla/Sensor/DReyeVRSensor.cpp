#include "Carla/Sensor/DReyeVRSensor.h"
#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Game/CarlaEpisode.h"

#include <sstream>
#include <string>

// vector types for serialization
#include "carla/geom/Vector2D.h"
#include "carla/geom/Vector3D.h"

struct DReyeVR::SensorData *ADReyeVRSensor::Snapshot = nullptr;

ADReyeVRSensor::ADReyeVRSensor(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    // no need for any other initialization
    PrimaryActorTick.bCanEverTick = true;
    Snapshot = new struct DReyeVR::SensorData;
}

FActorDefinition ADReyeVRSensor::GetSensorDefinition()
{
    // What our sensor is going to be called:
    auto Definition = UActorBlueprintFunctionLibrary::MakeGenericSensorDefinition(
        TEXT("DReyeVR"),        // folder directory   "dreyevr"
        TEXT("DReyeVRSensor")); // sensor name        "dreyevrsensor"

    /// NOTE: only has EActorAttributeType for bool, int, float, string, and RGBColor
    // see /Plugins/Carla/Source/Carla/Actor/ActorAttribute.h for the whole list, not much use though

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
}

void ADReyeVRSensor::BeginDestroy()
{
    Super::BeginDestroy();
}

carla::geom::Vector3D FVectorToGeom(const FVector &In)
{
    return carla::geom::Vector3D{In.X, In.Y, In.Z};
}
carla::geom::Vector3D FRotatorToGeom(const FRotator &In)
{
    return carla::geom::Vector3D{In.Pitch, In.Roll, In.Yaw};
}
carla::geom::Vector2D FVector2DToGeom2D(const FVector2D &In)
{
    return carla::geom::Vector2D{In.X, In.Y};
}
void ADReyeVRSensor::PostPhysTick(UWorld *W, ELevelTick TickType, float DeltaSeconds)
{
    // REQUIRES a client to be connected to stream to
    if (ClientInitialized)
    {
        auto Stream = GetDataStream(*this);
        Stream.Send(*this,
                    Snapshot->TimestampSR,                             // Timestamp of SRanipal (ms)
                    Snapshot->TimestampCarla,                          // Timestamp of Carla (ms)
                    Snapshot->FrameSequence,                           // Frame sequence
                    FVectorToGeom(Snapshot->Combined.GazeRay),         // Stream GazeRay Vec3
                    FVectorToGeom(Snapshot->Combined.Origin),          // Stream EyeOrigin Vec3
                    Snapshot->Combined.GazeValid,                      // Validity of combined gaze
                    Snapshot->Combined.Vergence,                       // Vergence (float) of combined ray
                    FVectorToGeom(Snapshot->HMDLocation),              // HMD absolute location
                    FRotatorToGeom(Snapshot->HMDRotation),             // HMD absolute rotation
                    FVectorToGeom(Snapshot->Left.GazeRay),             // Left eye gaze ray
                    FVectorToGeom(Snapshot->Left.Origin),              // left eye origin
                    Snapshot->Left.GazeValid,                          // Validity of left gaze
                    FVectorToGeom(Snapshot->Right.GazeRay),            // right eye gaze ray
                    FVectorToGeom(Snapshot->Right.Origin),             // right eye origin
                    Snapshot->Right.GazeValid,                         // Validity of right gaze
                    Snapshot->Left.EyeOpenness,                        // left eye openness
                    Snapshot->Left.EyeOpenValid,                       // Validity of left eye openness
                    Snapshot->Right.EyeOpenness,                       // right eye openness
                    Snapshot->Right.EyeOpenValid,                      // Validity of right eye openness
                    FVector2DToGeom2D(Snapshot->Left.PupilPos),        // left pupil position
                    Snapshot->Left.PupilPosValid,                      // Validity of left eye posn
                    FVector2DToGeom2D(Snapshot->Right.PupilPos),       // right pupil position
                    Snapshot->Right.PupilPosValid,                     // Validity of left eye posn
                    Snapshot->Left.PupilDiam,                          // Left eye diameter (mm)
                    Snapshot->Right.PupilDiam,                         // Right eye diameter (mm)
                    carla::rpc::FromFString(Snapshot->FocusActorName), // Focus Actor's name
                    FVectorToGeom(Snapshot->FocusActorPoint),          // Focus Actor's location in world space
                    Snapshot->FocusActorDist,                          // Focus Actor's distance to the sensor
                    Snapshot->Inputs.Throttle,                         // Vehicle input throttle
                    Snapshot->Inputs.Steering,                         // Vehicle input steering
                    Snapshot->Inputs.Brake,                            // Vehicle input brake
                    Snapshot->Inputs.ToggledReverse,                   // Vehicle input gear (reverse, fwd)
                    Snapshot->Inputs.HoldHandbrake                     // Vehicle input handbrake
        );
    }
}

void ADReyeVRSensor::SetIsReplaying(const bool Replaying)
{
    IsReplaying = Replaying;
}

bool ADReyeVRSensor::GetIsReplaying()
{
    return IsReplaying;
}

void ADReyeVRSensor::Update(const DReyeVR::SensorData *NewData)
{
    // update the static struct with incoming values
    (*Snapshot) = (*NewData);
}

/////////////////////////:DREYEVRSENSORDATA://////////////////////////////
/// NOTE: this to define the static variables that are set by the replayer when
//  replaying files to animate gaze ray data
bool ADReyeVRSensor::IsReplaying = false; // not replaying initially
FTransform ADReyeVRSensor::EgoReplayTransform = FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(1, 1, 1));
float ADReyeVRSensor::EgoReplayVelocity = 0;

void ADReyeVRSensor::UpdateReplayData(const DReyeVR::SensorData &R_Snapshot, const FTransform &EgoTrans,
                                      const double Per)
{
    // updates a bunch of local values
    ADReyeVRSensor::SetIsReplaying(true);
    ADReyeVRSensor::EgoReplayTransform = EgoTrans;
    ADReyeVRSensor::EgoReplayVelocity = R_Snapshot.Velocity;
    if (ADReyeVRSensor::Snapshot != nullptr)
    {
        FVector NewCameraPose;
        FRotator NewCameraRot;
        ADReyeVRSensor::InterpPositionAndRotation(ADReyeVRSensor::Snapshot->HMDLocation, // old location
                                                  ADReyeVRSensor::Snapshot->HMDRotation, // old rotation
                                                  R_Snapshot.HMDLocation,                // new location
                                                  R_Snapshot.HMDRotation,                // new rotation
                                                  Per, NewCameraPose, NewCameraRot);
        // apply most of the update, but will need to assign the new values next
        (*ADReyeVRSensor::Snapshot) = R_Snapshot;
        // assign new values
        ADReyeVRSensor::Snapshot->HMDLocation = NewCameraPose;
        ADReyeVRSensor::Snapshot->HMDRotation = NewCameraRot;
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
