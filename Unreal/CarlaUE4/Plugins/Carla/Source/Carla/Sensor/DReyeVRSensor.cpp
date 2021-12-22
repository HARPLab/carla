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

struct DReyeVR::SensorData *ADReyeVRSensor::Data = nullptr;

ADReyeVRSensor::ADReyeVRSensor(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    // no need for any other initialization
    PrimaryActorTick.bCanEverTick = true;
    ADReyeVRSensor::Data = new struct DReyeVR::SensorData;
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

    UCarlaGameInstance *CarlaGame = UCarlaStatics::GetGameInstance(World);
    SetEpisode(*(CarlaGame->GetCarlaEpisode()));
    SetDataStream(CarlaGame->GetServer().OpenStream()); // initialize boost::optional<Stream>
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
    if (!this->StreamSensorData) // param for enabling or disabling the data streaming
        return;
    auto Stream = GetDataStream(*this);
    DReyeVR::SensorData *Snapshot = this->GetData();
    auto &SR_Combo = Snapshot->Combined;
    auto &SR_Right = Snapshot->Right;
    auto &SR_Left = Snapshot->Left;
    /// TODO: refactor this somehow (add param to keep stream online or not)
    Stream.Send(*this,
                Snapshot->TimestampSR,                             // Timestamp of SRanipal (ms)
                Snapshot->TimestampCarla,                          // Timestamp of Carla (ms)
                Snapshot->FrameSequence,                           // Frame sequence
                FVectorToGeom(SR_Combo.GazeRay),                   // Stream GazeRay Vec3
                FVectorToGeom(SR_Combo.Origin),                    // Stream EyeOrigin Vec3
                SR_Combo.GazeValid,                                // Validity of combined gaze
                SR_Combo.Vergence,                                 // Vergence (float) of combined ray
                FVectorToGeom(Snapshot->HMDLocation),              // HMD absolute location
                FRotatorToGeom(Snapshot->HMDRotation),             // HMD absolute rotation
                FVectorToGeom(SR_Left.GazeRay),                    // Left eye gaze ray
                FVectorToGeom(SR_Left.Origin),                     // left eye origin
                SR_Left.GazeValid,                                 // Validity of left gaze
                FVectorToGeom(SR_Right.GazeRay),                   // right eye gaze ray
                FVectorToGeom(SR_Right.Origin),                    // right eye origin
                SR_Right.GazeValid,                                // Validity of right gaze
                SR_Left.EyeOpenness,                               // left eye openness
                SR_Left.EyeOpenValid,                              // Validity of left eye openness
                SR_Right.EyeOpenness,                              // right eye openness
                SR_Right.EyeOpenValid,                             // Validity of right eye openness
                FVector2DToGeom2D(SR_Left.PupilPos),               // left pupil position
                SR_Left.PupilPosValid,                             // Validity of left eye posn
                FVector2DToGeom2D(SR_Right.PupilPos),              // right pupil position
                SR_Right.PupilPosValid,                            // Validity of left eye posn
                SR_Left.PupilDiam,                                 // Left eye diameter (mm)
                SR_Right.PupilDiam,                                // Right eye diameter (mm)
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

/////////////////////////:DREYEVRSENSORDATA://////////////////////////////
/// NOTE: this to define the static variables that are set by the replayer when
//  replaying files to animate gaze ray data
bool ADReyeVRSensor::bIsReplaying = false; // not replaying initially
FTransform ADReyeVRSensor::EgoReplayTransform = FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(1, 1, 1));
float ADReyeVRSensor::EgoReplayVelocity = 0;

void ADReyeVRSensor::UpdateReplayData(const DReyeVR::SensorData &RecorderData, const FTransform &EgoTrans,
                                      const double Per)
{
    // update global values
    ADReyeVRSensor::bIsReplaying = true;
    ADReyeVRSensor::EgoReplayTransform = EgoTrans;
    ADReyeVRSensor::EgoReplayVelocity = RecorderData.Velocity;
    if (ADReyeVRSensor::Data != nullptr)
    {
        // update local values
        FVector NewCameraPose;
        FRotator NewCameraRot;
        ADReyeVRSensor::InterpPositionAndRotation(ADReyeVRSensor::Data->HMDLocation, // old location
                                                  ADReyeVRSensor::Data->HMDRotation, // old rotation
                                                  RecorderData.HMDLocation,          // new location
                                                  RecorderData.HMDRotation,          // new rotation
                                                  Per, NewCameraPose, NewCameraRot);
        // apply most of the update, but will need to assign the new values next
        (*ADReyeVRSensor::Data) = RecorderData;
        // assign new values
        ADReyeVRSensor::Data->HMDLocation = NewCameraPose;
        ADReyeVRSensor::Data->HMDRotation = NewCameraRot;
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
