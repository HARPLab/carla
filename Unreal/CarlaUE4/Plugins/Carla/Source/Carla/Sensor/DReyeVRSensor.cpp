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
    /// NOTE: this function defines the routine for streaming data to the PythonAPI
    if (!this->StreamSensorData) // param for enabling or disabling the data streaming
        return;
    auto Stream = GetDataStream(*this);
    const DReyeVR::SensorData *AllData = this->GetData();
    /// TODO: refactor this somehow
    Stream.Send(*this,
                AllData->GetTimestampDevice(),                         // Timestamp of SRanipal (ms)
                AllData->GetTimestampCarla(),                          // Timestamp of Carla (ms)
                AllData->GetFrameSequence(),                           // Frame sequence
                FVectorToGeom(AllData->GetCombinedGazeDir()),          // Combined gaze ray direction
                FVectorToGeom(AllData->GetCombinedGazeOrigin()),       // Stream EyeOrigin Vec3
                AllData->GetCombinedValidity(),                        // Validity of combined gaze
                AllData->GetEyeVergence(),                             // Vergence (float) of combined ray
                FVectorToGeom(AllData->GetHMDLocation()),              // HMD absolute location
                FRotatorToGeom(AllData->GetHMDRotation()),             // HMD absolute rotation
                FVectorToGeom(AllData->GetLeftGazeDir()),              // Left eye gaze ray direction
                FVectorToGeom(AllData->GetLeftGazeOrigin()),           // Left eye gaze origin
                AllData->GetLeftValidity(),                            // Validity of left gaze
                FVectorToGeom(AllData->GetRightGazeDir()),             // Right eye gaze ray direction
                FVectorToGeom(AllData->GetRightGazeOrigin()),          // Dight eye gaze origin
                AllData->GetRightValidity(),                           // Validity of right gaze
                AllData->GetLeftEyeOpenness(),                         // Left eye openness
                AllData->GetLeftEyeOpennessValidity(),                 // Validity of left eye openness
                AllData->GetRightEyeOpenness(),                        // Right eye openness
                AllData->GetRightEyeOpennessValidity(),                // Validity of right eye openness
                FVector2DToGeom2D(AllData->GetLeftPupilPosition()),    // Left pupil position
                AllData->GetLeftPupilPositionValidity(),               // Validity of left eye posn
                FVector2DToGeom2D(AllData->GetRightPupilPosition()),   // Right pupil position
                AllData->GetRightPupilPositionValidity(),              // Validity of left eye posn
                AllData->GetLeftPupilDiameter(),                       // Left eye diameter (mm)
                AllData->GetRightPupilDiameter(),                      // Right eye diameter (mm)
                carla::rpc::FromFString(AllData->GetFocusActorName()), // Focus Actor's name
                FVectorToGeom(AllData->GetFocusActorPoint()),          // Focus Actor's location in world space
                AllData->GetFocusActorDistance(),                      // Focus Actor's distance to the sensor
                AllData->GetUserInputs().Throttle,                     // Vehicle input throttle
                AllData->GetUserInputs().Steering,                     // Vehicle input steering
                AllData->GetUserInputs().Brake,                        // Vehicle input brake
                AllData->GetUserInputs().ToggledReverse,               // Vehicle input gear (reverse, fwd)
                AllData->GetUserInputs().HoldHandbrake                 // Vehicle input handbrake
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
    ADReyeVRSensor::EgoReplayVelocity = RecorderData.GetEgoVelocity();
    if (ADReyeVRSensor::Data != nullptr)
    {
        // update local values
        FVector NewCameraPose;
        FRotator NewCameraRot;
        ADReyeVRSensor::InterpPositionAndRotation(ADReyeVRSensor::Data->GetHMDLocation(), // old location
                                                  ADReyeVRSensor::Data->GetHMDRotation(), // old rotation
                                                  RecorderData.GetHMDLocation(),          // new location
                                                  RecorderData.GetHMDRotation(),          // new rotation
                                                  Per, NewCameraPose, NewCameraRot);
        // apply most of the update, but will need to assign the new values next
        (*ADReyeVRSensor::Data) = RecorderData;
        // assign new values
        ADReyeVRSensor::Data->GetHMDLocation() = NewCameraPose;
        ADReyeVRSensor::Data->GetHMDRotation() = NewCameraRot;
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
