#pragma once

#include "carla/geom/Vector3D.h"
#include "carla/sensor/SensorData.h"
#include "carla/sensor/s11n/DReyeVRSerializer.h"

#include <cstdint>

namespace carla
{
namespace rpc
{
using Float = float;
using Bool = bool;
using Int = int;
using Int64 = int64_t;
using String = std::string;
} // namespace rpc

namespace sensor
{
namespace data
{
class DReyeVREvent : public SensorData
{
    using Super = SensorData;
    using Serializer = s11n::DReyeVRSerializer;
    friend Serializer;

  protected:
    explicit DReyeVREvent(const RawData &data) : Super(data)
    {
        auto Deserialized = Serializer::DeserializeRawData(data);
        TimestampSR = Deserialized.TimestampSR;
        TimestampCarla = Deserialized.TimestampCarla;
        FrameSequence = Deserialized.FrameSequence;
        GazeRay = Deserialized.GazeRay;
        EyeOrigin = Deserialized.EyeOrigin;
        GazeValid = Deserialized.GazeValid;
        Vergence = Deserialized.Vergence;
        CameraLocation = Deserialized.CameraLocation;
        CameraRotation = Deserialized.CameraRotation;
        LGazeRay = Deserialized.LGazeRay;
        LEyeOrigin = Deserialized.LEyeOrigin;
        LGazeRay = Deserialized.LGazeRay;
        RGazeRay = Deserialized.RGazeRay;
        REyeOrigin = Deserialized.REyeOrigin;
        RGazeRay = Deserialized.RGazeRay;
        LEyeOpenness = Deserialized.LEyeOpenness;
        LEyeOpenValid = Deserialized.LEyeOpenValid;
        REyeOpenness = Deserialized.REyeOpenness;
        REyeOpenValid = Deserialized.REyeOpenValid;
        LPupilPos = Deserialized.LPupilPos;
        LPupilPosValid = Deserialized.LPupilPosValid;
        RPupilPos = Deserialized.RPupilPos;
        LPupilDiameter = Deserialized.LPupilDiameter;
        RPupilDiameter = Deserialized.RPupilDiameter;
        RPupilPosValid = Deserialized.RPupilPosValid;
        FocusActorName = Deserialized.FocusActorName;
        FocusActorPoint = Deserialized.FocusActorPoint;
        FocusActorDist = Deserialized.FocusActorDist;
        Throttle = Deserialized.Throttle;
        Steering = Deserialized.Steering;
        Brake = Deserialized.Brake;
        ToggledReverse = Deserialized.ToggledReverse;
        HoldHandbrake = Deserialized.HoldHandbrake;
    }

  public:
    const rpc::Int64 &GetSRTimestamp() const
    {
        return TimestampSR;
    }
    const rpc::Int64 &GetCarlaTimestamp() const
    {
        return TimestampCarla;
    }
    const rpc::Int64 &GetFrameSequence() const
    {
        return FrameSequence;
    }
    const geom::Vector3D &GetGazeRay() const
    {
        return GazeRay;
    }
    const geom::Vector3D &GetEyeOrigin() const
    {
        return EyeOrigin;
    }
    const rpc::Bool &GetGazeValid() const
    {
        return GazeValid;
    }
    const rpc::Float &GetVergence() const
    {
        return Vergence;
    }
    const geom::Vector3D &GetCameraLocation() const
    {
        return CameraLocation;
    }
    const geom::Vector3D &GetCameraRotation() const
    {
        return CameraRotation;
    }
    const geom::Vector3D &GetLGazeRay() const
    {
        return LGazeRay;
    }
    const geom::Vector3D &GetLEyeOrigin() const
    {
        return LEyeOrigin;
    }
    const rpc::Bool &GetLGazeValid() const
    {
        return LGazeValid;
    }
    const geom::Vector3D &GetRGazeRay() const
    {
        return RGazeRay;
    }
    const geom::Vector3D &GetREyeOrigin() const
    {
        return REyeOrigin;
    }
    const rpc::Bool &GetRGazeValid() const
    {
        return RGazeValid;
    }
    const rpc::Float &GetLEyeOpenness() const
    {
        return LEyeOpenness;
    }
    const rpc::Bool &GetLEyeOpenValid() const
    {
        return LEyeOpenValid;
    }
    const rpc::Float &GetREyeOpenness() const
    {
        return REyeOpenness;
    }
    const rpc::Bool &GetREyeOpenValid() const
    {
        return REyeOpenValid;
    }
    const geom::Vector2D &GetLPupilPos() const
    {
        return LPupilPos;
    }
    const rpc::Bool &GetLPupilPosValid() const
    {
        return LPupilPosValid;
    }
    const geom::Vector2D &GetRPupilPos() const
    {
        return RPupilPos;
    }
    const rpc::Bool &GetRPupilPosValid() const
    {
        return RPupilPosValid;
    }
    const rpc::Float &GetLPupilDiam() const
    {
        return LPupilDiameter;
    }
    const rpc::Float &GetRPupilDiam() const
    {
        return RPupilDiameter;
    }
    const rpc::String &GetFocusActorName() const
    {
        return FocusActorName;
    }
    const geom::Vector3D &GetFocusActorPoint() const
    {
        return FocusActorPoint;
    }
    const rpc::Float &GetFocusActorDist() const
    {
        return FocusActorDist;
    }
    const rpc::Float &GetThrottle() const
    {
        return Throttle;
    }
    const rpc::Float &GetSteering() const
    {
        return Steering;
    }
    const rpc::Float &GetBrake() const
    {
        return Brake;
    }
    const rpc::Bool &GetToggledReverse() const
    {
        return ToggledReverse;
    }
    const rpc::Bool &GetHandbrake() const
    {
        return HoldHandbrake;
    }

  private:
    rpc::Int64 TimestampSR;
    rpc::Int64 TimestampCarla;
    rpc::Int64 FrameSequence;
    geom::Vector3D GazeRay;
    geom::Vector3D EyeOrigin;
    rpc::Bool GazeValid;
    rpc::Float Vergence;
    geom::Vector3D CameraLocation;
    geom::Vector3D CameraRotation;
    geom::Vector3D LGazeRay;
    geom::Vector3D LEyeOrigin;
    rpc::Bool LGazeValid;
    geom::Vector3D RGazeRay;
    geom::Vector3D REyeOrigin;
    rpc::Bool RGazeValid;
    rpc::Float LEyeOpenness;
    rpc::Bool LEyeOpenValid;
    rpc::Float REyeOpenness;
    rpc::Bool REyeOpenValid;
    geom::Vector2D LPupilPos;
    rpc::Bool LPupilPosValid;
    geom::Vector2D RPupilPos;
    rpc::Bool RPupilPosValid;
    rpc::Float LPupilDiameter;
    rpc::Float RPupilDiameter;
    rpc::String FocusActorName;
    geom::Vector3D FocusActorPoint;
    rpc::Float FocusActorDist;
    rpc::Float Throttle;
    rpc::Float Steering;
    rpc::Float Brake;
    rpc::Bool ToggledReverse;
    rpc::Bool HoldHandbrake;
};
} // namespace data
} // namespace sensor
} // namespace carla