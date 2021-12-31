
#pragma once

#include "carla/Buffer.h"
#include "carla/Memory.h"
#include "carla/geom/Vector2D.h"
#include "carla/geom/Vector3D.h"
#include "carla/sensor/RawData.h"

#include <cstdint>
#include <string>

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
class SensorData;

namespace s11n
{
class DReyeVRSerializer
{
  public:
    struct Data
    {
    /// EW: this is gross but I am unable to think of a better way to do this short of defining our own rpc type
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
        // inputs
        rpc::Float Throttle;
        rpc::Float Steering;
        rpc::Float Brake;
        rpc::Bool ToggledReverse;
        rpc::Bool HoldHandbrake;

        MSGPACK_DEFINE_ARRAY(TimestampSR, TimestampCarla, FrameSequence, GazeRay, EyeOrigin, GazeValid, Vergence,
                             CameraLocation, CameraRotation, LGazeRay, LEyeOrigin, LGazeValid, RGazeRay, REyeOrigin,
                             RGazeValid, LEyeOpenness, LEyeOpenValid, REyeOpenness, REyeOpenValid, LPupilPos,
                             LPupilPosValid, RPupilPos, RPupilPosValid, LPupilDiameter, RPupilDiameter, FocusActorName,
                             FocusActorPoint, FocusActorDist, Throttle, Steering, Brake, ToggledReverse, HoldHandbrake)
    };

    static Data DeserializeRawData(const RawData &message)
    {
        return MsgPack::UnPack<Data>(message.begin(), message.size());
    }

    template <typename SensorT> static Buffer Serialize(const SensorT &, struct Data &&DataIn)
    {
        return MsgPack::Pack(DataIn);
    }
    static SharedPtr<SensorData> Deserialize(RawData &&data);
};

} // namespace s11n
} // namespace sensor
} // namespace carla