
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
        rpc::Int64 TimestampSR;
        rpc::Int64 TimestampCarla;
        rpc::Int64 FrameSequence;
        geom::Vector3D GazeRay;
        geom::Vector3D EyeOrigin;
        rpc::Bool GazeValid;
        rpc::Float Vergence;
        geom::Vector3D HMDLocation;
        geom::Vector3D HMDRotation;
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
                             HMDLocation, HMDRotation, LGazeRay, LEyeOrigin, LGazeValid, RGazeRay, REyeOrigin,
                             RGazeValid, LEyeOpenness, LEyeOpenValid, REyeOpenness, REyeOpenValid, LPupilPos,
                             LPupilPosValid, RPupilPos, RPupilPosValid, LPupilDiameter, RPupilDiameter, FocusActorName,
                             FocusActorPoint, FocusActorDist, Throttle, Steering, Brake, ToggledReverse, HoldHandbrake)
    };

    static Data DeserializeRawData(const RawData &message)
    {
        return MsgPack::UnPack<Data>(message.begin(), message.size());
    }

    template <typename SensorT>
    static Buffer Serialize(const SensorT &, const rpc::Int64 TimestampSR, const rpc::Int64 TimestampCarla,
                            const rpc::Int64 FrameSequence, const geom::Vector3D &GazeRay,
                            const geom::Vector3D &EyeOrigin, const rpc::Bool GazeValid, const rpc::Float Vergence,
                            const geom::Vector3D &HMDLocation, const geom::Vector3D &HMDRotation,
                            const geom::Vector3D &LGazeRay, const geom::Vector3D &LEyeOrigin,
                            const rpc::Bool LGazeValid, const geom::Vector3D &RGazeRay,
                            const geom::Vector3D &REyeOrigin, const rpc::Bool RGazeValid, const rpc::Float LEyeOpenness,
                            const rpc::Bool LEyeOpenValid, const rpc::Float REyeOpenness, const rpc::Bool REyeOpenValid,
                            const geom::Vector2D &LPupilPos, const rpc::Bool LPupilPosValid,
                            const geom::Vector2D &RPupilPos, const rpc::Bool RPupilPosValid,
                            const rpc::Float LPupilDiameter, const rpc::Float RPupilDiameter,
                            const rpc::String &FocusActorName, const geom::Vector3D &FocusActorPoint,
                            const rpc::Float FocusActorDist, const rpc::Float Throttle, const rpc::Float Steering,
                            const rpc::Float Brake, const rpc::Bool ToggledReverse, const rpc::Bool HoldHandbrake)
    {
        return MsgPack::Pack(Data{TimestampSR,    TimestampCarla,  FrameSequence,  GazeRay,        EyeOrigin,
                                  GazeValid,      Vergence,        HMDLocation,    HMDRotation,    LGazeRay,
                                  LEyeOrigin,     LGazeValid,      RGazeRay,       REyeOrigin,     RGazeValid,
                                  LEyeOpenness,   LEyeOpenValid,   REyeOpenness,   REyeOpenValid,  LPupilPos,
                                  LPupilPosValid, RPupilPos,       RPupilPosValid, LPupilDiameter, RPupilDiameter,
                                  FocusActorName, FocusActorPoint, FocusActorDist, Throttle,       Steering,
                                  Brake,          ToggledReverse,  HoldHandbrake});
    }
    static SharedPtr<SensorData> Deserialize(RawData &&data);
};

} // namespace s11n
} // namespace sensor
} // namespace carla