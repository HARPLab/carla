// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "DReyeVRRecorder.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

/// ========================================== ///
/// ---------------:READERS:------------------ ///
/// ========================================== ///
void DReyeVRDataRecorder::Read(std::ifstream &InFile)
{
    /// CAUTION: make sure the order of writes/reads is the same
    ReadValue<int64_t>(InFile, Data.TimestampCarla());
    ReadValue<int64_t>(InFile, Data.TimestampDevice());
    ReadValue<int64_t>(InFile, Data.FrameSequence());
    ReadValue<float>(InFile, Data.GazeVergence());
    ReadFVector(InFile, Data.GazeDir(DReyeVR::Gaze::COMBINED));
    ReadFVector(InFile, Data.GazeOrigin(DReyeVR::Gaze::COMBINED));
    ReadValue<bool>(InFile, Data.GazeValidity(DReyeVR::Gaze::COMBINED));
    ReadFVector(InFile, Data.GazeDir(DReyeVR::Gaze::LEFT));
    ReadFVector(InFile, Data.GazeOrigin(DReyeVR::Gaze::LEFT));
    ReadValue<bool>(InFile, Data.GazeValidity(DReyeVR::Gaze::LEFT));
    ReadValue<float>(InFile, Data.EyeOpenness(DReyeVR::Eye::LEFT));
    ReadValue<bool>(InFile, Data.EyeOpennessValidity(DReyeVR::Eye::LEFT));
    ReadValue<float>(InFile, Data.PupilDiameter(DReyeVR::Eye::LEFT));
    ReadFVector2D(InFile, Data.PupilPosition(DReyeVR::Eye::LEFT));
    ReadValue<bool>(InFile, Data.PupilPositionValidity(DReyeVR::Eye::LEFT));
    ReadFVector(InFile, Data.GazeDir(DReyeVR::Gaze::RIGHT));
    ReadFVector(InFile, Data.GazeOrigin(DReyeVR::Gaze::RIGHT));
    ReadValue<bool>(InFile, Data.GazeValidity(DReyeVR::Gaze::RIGHT));
    ReadValue<float>(InFile, Data.EyeOpenness(DReyeVR::Eye::RIGHT));
    ReadValue<bool>(InFile, Data.EyeOpennessValidity(DReyeVR::Eye::RIGHT));
    ReadValue<float>(InFile, Data.PupilDiameter(DReyeVR::Eye::RIGHT));
    ReadFVector2D(InFile, Data.PupilPosition(DReyeVR::Eye::RIGHT));
    ReadValue<bool>(InFile, Data.PupilPositionValidity(DReyeVR::Eye::RIGHT));
    ReadFVector(InFile, Data.HMDLocation());
    ReadFRotator(InFile, Data.HMDRotation());
    ReadValue<float>(InFile, Data.EgoVelocity());
    ReadFString(InFile, Data.FocusActorName());
    ReadFVector(InFile, Data.FocusActorPoint());
    ReadValue<float>(InFile, Data.FocusActorDistance());
    ReadValue<float>(InFile, Data.UserInputs().Throttle);
    ReadValue<float>(InFile, Data.UserInputs().Steering);
    ReadValue<float>(InFile, Data.UserInputs().Brake);
    ReadValue<bool>(InFile, Data.UserInputs().ToggledReverse);
    ReadValue<bool>(InFile, Data.UserInputs().TurnSignalLeft);
    ReadValue<bool>(InFile, Data.UserInputs().TurnSignalRight);
    ReadValue<bool>(InFile, Data.UserInputs().HoldHandbrake);
}

/// ========================================== ///
/// ---------------:WRITERS:------------------ ///
/// ========================================== ///

void DReyeVRDataRecorder::Write(std::ofstream &OutFile) const
{
    /// CAUTION: make sure the order of writes/reads is the same
    WriteValue<int64_t>(OutFile, Data.TimestampCarla());
    WriteValue<int64_t>(OutFile, Data.TimestampDevice());
    WriteValue<int64_t>(OutFile, Data.FrameSequence());
    WriteValue<float>(OutFile, Data.GazeVergence());
    WriteFVector(OutFile, Data.GazeDir(DReyeVR::Gaze::COMBINED));
    WriteFVector(OutFile, Data.GazeOrigin(DReyeVR::Gaze::COMBINED));
    WriteValue<bool>(OutFile, Data.GazeValidity(DReyeVR::Gaze::COMBINED));
    WriteFVector(OutFile, Data.GazeDir(DReyeVR::Gaze::LEFT));
    WriteFVector(OutFile, Data.GazeOrigin(DReyeVR::Gaze::LEFT));
    WriteValue<bool>(OutFile, Data.GazeValidity(DReyeVR::Gaze::LEFT));
    WriteValue<float>(OutFile, Data.EyeOpenness(DReyeVR::Eye::LEFT));
    WriteValue<bool>(OutFile, Data.EyeOpennessValidity(DReyeVR::Eye::LEFT));
    WriteValue<float>(OutFile, Data.PupilDiameter(DReyeVR::Eye::LEFT));
    WriteFVector2D(OutFile, Data.PupilPosition(DReyeVR::Eye::LEFT));
    WriteValue<bool>(OutFile, Data.PupilPositionValidity(DReyeVR::Eye::LEFT));
    WriteFVector(OutFile, Data.GazeDir(DReyeVR::Gaze::RIGHT));
    WriteFVector(OutFile, Data.GazeOrigin(DReyeVR::Gaze::RIGHT));
    WriteValue<bool>(OutFile, Data.GazeValidity(DReyeVR::Gaze::RIGHT));
    WriteValue<float>(OutFile, Data.EyeOpenness(DReyeVR::Eye::RIGHT));
    WriteValue<bool>(OutFile, Data.EyeOpennessValidity(DReyeVR::Eye::RIGHT));
    WriteValue<float>(OutFile, Data.PupilDiameter(DReyeVR::Eye::RIGHT));
    WriteFVector2D(OutFile, Data.PupilPosition(DReyeVR::Eye::RIGHT));
    WriteValue<bool>(OutFile, Data.PupilPositionValidity(DReyeVR::Eye::RIGHT));
    WriteFVector(OutFile, Data.HMDLocation());
    WriteFRotator(OutFile, Data.HMDRotation());
    WriteValue<float>(OutFile, Data.EgoVelocity());
    WriteFString(OutFile, Data.FocusActorName());
    WriteFVector(OutFile, Data.FocusActorPoint());
    WriteValue<float>(OutFile, Data.FocusActorDistance());
    WriteValue<float>(OutFile, Data.UserInputs().Throttle);
    WriteValue<float>(OutFile, Data.UserInputs().Steering);
    WriteValue<float>(OutFile, Data.UserInputs().Brake);
    WriteValue<bool>(OutFile, Data.UserInputs().ToggledReverse);
    WriteValue<bool>(OutFile, Data.UserInputs().TurnSignalLeft);
    WriteValue<bool>(OutFile, Data.UserInputs().TurnSignalRight);
    WriteValue<bool>(OutFile, Data.UserInputs().HoldHandbrake);
}

/// ========================================== ///
/// ---------------:PRINTING:----------------- ///
/// ========================================== ///

inline std::string VecToString(const FVector &X)
{
    std::ostringstream oss;
    oss << "{" << X.X << "," << X.Y << "," << X.Z << "}";
    return oss.str();
}

inline std::string VecToString(const FVector2D &X)
{
    std::ostringstream oss;
    oss << "{" << X.X << "," << X.Y << "}";
    return oss.str();
}

inline std::string VecToString(const FRotator &X)
{
    std::ostringstream oss;
    oss << "{" << X.Pitch << "," << X.Roll << "," << X.Yaw << "}";
    return oss.str();
}

std::string DReyeVRDataRecorder::Print() const
{
    std::ostringstream oss;

    const std::string sep = "; ";                                                       // semicolon + space sepiter
    oss << "T_SRanipal: " << Data.TimestampDevice() << sep                              // Sranipal time
        << "T_Carla: " << Data.TimestampCarla() << sep                                  // Carla time
        << "FrameSeq: " << Data.FrameSequence() << sep                                  // SRanipal Framesequence
        << "CGazeDir: " << VecToString(Data.GazeDir()) << sep                           // Gaze ray vector
        << "CGazeOrigin: " << VecToString(Data.GazeOrigin()) << sep                     // Combined Eye origin vector
        << "Vergence: " << Data.GazeVergence() << sep                                   // Calculated vergence
        << "HMDLoc: " << VecToString(Data.HMDLocation()) << sep                         // HMD location
        << "HMDRot: " << VecToString(Data.HMDRotation()) << sep                         // HMD rotation
        << "EgoVel: " << Data.EgoVelocity() << sep                                      // Ego Velocity
        << "LGazeDir: " << VecToString(Data.GazeDir(DReyeVR::Gaze::LEFT)) << sep        // LEFT gaze ray
        << "LEyeOrigin: " << VecToString(Data.GazeOrigin(DReyeVR::Gaze::LEFT)) << sep   // LEFT gaze ray
        << "RGazeDir: " << VecToString(Data.GazeDir(DReyeVR::Gaze::RIGHT)) << sep       // RIGHT gaze ray
        << "REyeOrigin: " << VecToString(Data.GazeOrigin(DReyeVR::Gaze::RIGHT)) << sep  // RIGHT gaze ray
        << "LEyeOpenness: " << Data.EyeOpenness(DReyeVR::Eye::LEFT) << sep              // LEFT eye openness
        << "REyeOpenness: " << Data.EyeOpenness(DReyeVR::Eye::RIGHT) << sep             // RIGHT eye openness
        << "LPupilPos: " << VecToString(Data.PupilPosition(DReyeVR::Eye::LEFT)) << sep  // LEFT pupil position
        << "RPupilPos: " << VecToString(Data.PupilPosition(DReyeVR::Eye::RIGHT)) << sep // RIGHT pupil position
        << "LPupilDiam: " << Data.PupilDiameter(DReyeVR::Eye::LEFT) << sep              // LEFT pupil diameter
        << "RPupilDiam: " << Data.PupilDiameter(DReyeVR::Eye::RIGHT) << sep             // LEFT pupil diameter
        << "FActorName: " << TCHAR_TO_UTF8(*Data.FocusActorName()) << sep               // Name (tag) of focus actor
        << "FActorPoint: " << VecToString(Data.FocusActorPoint()) << sep                // Location of focused actor
        << "FActorDist: " << Data.FocusActorDistance() << sep                           // Distance to focused actor
        << "VALIDITY: "                                                                 // validity booleans
        << "GazeV: " << Data.GazeValidity() << sep                                      // validity for COMBINE gaze
        << "LGazeV: " << Data.GazeValidity(DReyeVR::Gaze::LEFT) << sep                  // validity for LEFT gaze
        << "RGazeV: " << Data.GazeValidity(DReyeVR::Gaze::RIGHT) << sep                 // validity for RIGHT gaze
        << "LEyeOpenV: " << Data.EyeOpennessValidity(DReyeVR::Eye::LEFT) << sep         // validity for LEFT openness
        << "REyeOpenV: " << Data.EyeOpennessValidity(DReyeVR::Eye::RIGHT) << sep        // validity for RIGHT openness
        << "LPupilPosV: " << Data.PupilPositionValidity(DReyeVR::Eye::LEFT) << sep      // validity for LEFT pupil pos
        << "RPupilPosV: " << Data.PupilPositionValidity(DReyeVR::Eye::RIGHT) << sep     // validity for RIGHT pupil pos
        << "INPUTS: "                                                                   // User inputs
        << "Throttle: " << Data.UserInputs().Throttle << sep                            // Value of Throttle
        << "Steering: " << Data.UserInputs().Steering << sep                            // Value of Steering
        << "Brake: " << Data.UserInputs().Brake << sep                                  // Value of brake
        << "ToggleRev: " << Data.UserInputs().ToggledReverse << sep                     // toggled reverse
        << "TurnSignalLeft: " << Data.UserInputs().TurnSignalLeft << sep                // enabled left turn signal
        << "TurnSignalRight: " << Data.UserInputs().TurnSignalRight << sep              // enabled right turn signal
        << "Handbrake: " << Data.UserInputs().HoldHandbrake << sep                      // the handbrake is held
        ;
    return oss.str();
}
//---------------------------------------------

void DReyeVRDataRecorders::Clear(void)
{
    AllData.clear();
}

void DReyeVRDataRecorders::Add(const DReyeVRDataRecorder &NewData)
{
    AllData.push_back(NewData);
}

void DReyeVRDataRecorders::Write(std::ofstream &OutFile)
{
    // write the packet id
    WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::DReyeVR));
    std::streampos PosStart = OutFile.tellp();

    // write a dummy packet size
    uint32_t Total = 0;
    WriteValue<uint32_t>(OutFile, Total);

    // write total records
    Total = AllData.size();
    WriteValue<uint16_t>(OutFile, Total);

    for (auto &Snapshot : AllData)
        Snapshot.Write(OutFile);

    /// TODO: check if we need this? or can just write Total * sizeof(DReyeVRData)

    // write the real packet size
    std::streampos PosEnd = OutFile.tellp();
    Total = PosEnd - PosStart - sizeof(uint32_t);
    OutFile.seekp(PosStart, std::ios::beg);
    WriteValue<uint32_t>(OutFile, Total);
    OutFile.seekp(PosEnd, std::ios::beg);
}