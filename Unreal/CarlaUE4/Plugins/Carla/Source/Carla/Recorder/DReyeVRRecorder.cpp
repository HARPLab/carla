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
    ReadValue<int64_t>(InFile, Data.GetTimestampCarla());
    ReadValue<int64_t>(InFile, Data.GetTimestampDevice());
    ReadValue<int64_t>(InFile, Data.GetFrameSequence());
    ReadValue<float>(InFile, Data.GetEyeVergence());
    ReadFVector(InFile, Data.GetCombinedGazeDir());
    ReadFVector(InFile, Data.GetCombinedGazeOrigin());
    ReadValue<bool>(InFile, Data.GetCombinedValidity());
    ReadFVector(InFile, Data.GetLeftGazeDir());
    ReadFVector(InFile, Data.GetLeftGazeOrigin());
    ReadValue<bool>(InFile, Data.GetLeftValidity());
    ReadValue<float>(InFile, Data.GetLeftEyeOpenness());
    ReadValue<bool>(InFile, Data.GetLeftEyeOpennessValidity());
    ReadValue<float>(InFile, Data.GetLeftPupilDiameter());
    ReadFVector2D(InFile, Data.GetLeftPupilPosition());
    ReadValue<bool>(InFile, Data.GetLeftPupilPositionValidity());
    ReadFVector(InFile, Data.GetRightGazeDir());
    ReadFVector(InFile, Data.GetRightGazeOrigin());
    ReadValue<bool>(InFile, Data.GetRightValidity());
    ReadValue<float>(InFile, Data.GetRightEyeOpenness());
    ReadValue<bool>(InFile, Data.GetRightEyeOpennessValidity());
    ReadValue<float>(InFile, Data.GetRightPupilDiameter());
    ReadFVector2D(InFile, Data.GetRightPupilPosition());
    ReadValue<bool>(InFile, Data.GetRightPupilPositionValidity());
    ReadFVector(InFile, Data.GetHMDLocation());
    ReadFRotator(InFile, Data.GetHMDRotation());
    ReadValue<float>(InFile, Data.GetEgoVelocity());
    ReadFString(InFile, Data.GetFocusActorName());
    ReadFVector(InFile, Data.GetFocusActorPoint());
    ReadValue<float>(InFile, Data.GetFocusActorDistance());
    ReadValue<float>(InFile, Data.GetUserInputs().Throttle);
    ReadValue<float>(InFile, Data.GetUserInputs().Steering);
    ReadValue<float>(InFile, Data.GetUserInputs().Brake);
    ReadValue<bool>(InFile, Data.GetUserInputs().ToggledReverse);
    ReadValue<bool>(InFile, Data.GetUserInputs().TurnSignalLeft);
    ReadValue<bool>(InFile, Data.GetUserInputs().TurnSignalRight);
    ReadValue<bool>(InFile, Data.GetUserInputs().HoldHandbrake);
}

/// ========================================== ///
/// ---------------:WRITERS:------------------ ///
/// ========================================== ///

void DReyeVRDataRecorder::Write(std::ofstream &OutFile) const
{
    /// CAUTION: make sure the order of writes/reads is the same
    WriteValue<int64_t>(OutFile, Data.GetTimestampCarla());
    WriteValue<int64_t>(OutFile, Data.GetTimestampDevice());
    WriteValue<int64_t>(OutFile, Data.GetFrameSequence());
    WriteValue<float>(OutFile, Data.GetEyeVergence());
    WriteFVector(OutFile, Data.GetCombinedGazeDir());
    WriteFVector(OutFile, Data.GetCombinedGazeOrigin());
    WriteValue<bool>(OutFile, Data.GetCombinedValidity());
    WriteFVector(OutFile, Data.GetLeftGazeDir());
    WriteFVector(OutFile, Data.GetLeftGazeOrigin());
    WriteValue<bool>(OutFile, Data.GetLeftValidity());
    WriteValue<float>(OutFile, Data.GetLeftEyeOpenness());
    WriteValue<bool>(OutFile, Data.GetLeftEyeOpennessValidity());
    WriteValue<float>(OutFile, Data.GetLeftPupilDiameter());
    WriteFVector2D(OutFile, Data.GetLeftPupilPosition());
    WriteValue<bool>(OutFile, Data.GetLeftPupilPositionValidity());
    WriteFVector(OutFile, Data.GetRightGazeDir());
    WriteFVector(OutFile, Data.GetRightGazeOrigin());
    WriteValue<bool>(OutFile, Data.GetRightValidity());
    WriteValue<float>(OutFile, Data.GetRightEyeOpenness());
    WriteValue<bool>(OutFile, Data.GetRightEyeOpennessValidity());
    WriteValue<float>(OutFile, Data.GetRightPupilDiameter());
    WriteFVector2D(OutFile, Data.GetRightPupilPosition());
    WriteValue<bool>(OutFile, Data.GetRightPupilPositionValidity());
    WriteFVector(OutFile, Data.GetHMDLocation());
    WriteFRotator(OutFile, Data.GetHMDRotation());
    WriteValue<float>(OutFile, Data.GetEgoVelocity());
    WriteFString(OutFile, Data.GetFocusActorName());
    WriteFVector(OutFile, Data.GetFocusActorPoint());
    WriteValue<float>(OutFile, Data.GetFocusActorDistance());
    WriteValue<float>(OutFile, Data.GetUserInputs().Throttle);
    WriteValue<float>(OutFile, Data.GetUserInputs().Steering);
    WriteValue<float>(OutFile, Data.GetUserInputs().Brake);
    WriteValue<bool>(OutFile, Data.GetUserInputs().ToggledReverse);
    WriteValue<bool>(OutFile, Data.GetUserInputs().TurnSignalLeft);
    WriteValue<bool>(OutFile, Data.GetUserInputs().TurnSignalRight);
    WriteValue<bool>(OutFile, Data.GetUserInputs().HoldHandbrake);
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

    const std::string sep = "; ";                                              // semicolon + space sepiter
    oss << "T_SRanipal: " << Data.GetTimestampDevice() << sep                  // Sranipal time
        << "T_Carla: " << Data.GetTimestampCarla() << sep                      // Carla time
        << "FrameSeq: " << Data.GetFrameSequence() << sep                      // SRanipal Framesequence
        << "CGazeDir: " << VecToString(Data.GetCombinedGazeDir()) << sep       // Gaze ray vector
        << "CGazeOrigin: " << VecToString(Data.GetCombinedGazeOrigin()) << sep // Combined Eye origin vector
        << "Vergence: " << Data.GetEyeVergence() << sep                        // Calculated vergence
        << "HMDLoc: " << VecToString(Data.GetHMDLocation()) << sep             // HMD location
        << "HMDRot: " << VecToString(Data.GetHMDRotation()) << sep             // HMD rotation
        << "EgoVel: " << Data.GetEgoVelocity() << sep                          // Ego Velocity
        << "LGazeDir: " << VecToString(Data.GetLeftGazeDir()) << sep           // LEFT gaze ray
        << "LEyeOrigin: " << VecToString(Data.GetLeftGazeOrigin()) << sep      // LEFT gaze ray
        << "RGazeDir: " << VecToString(Data.GetRightGazeDir()) << sep          // RIGHT gaze ray
        << "REyeOrigin: " << VecToString(Data.GetRightGazeOrigin()) << sep     // RIGHT gaze ray
        << "LEyeOpenness: " << Data.GetLeftEyeOpenness() << sep                // LEFT eye openness
        << "REyeOpenness: " << Data.GetRightEyeOpenness() << sep               // RIGHT eye openness
        << "LPupilPos: " << VecToString(Data.GetLeftPupilPosition()) << sep    // LEFT pupil position
        << "RPupilPos: " << VecToString(Data.GetRightPupilPosition()) << sep   // RIGHT pupil position
        << "LPupilDiam: " << Data.GetLeftPupilDiameter() << sep                // LEFT pupil diameter
        << "RPupilDiam: " << Data.GetRightPupilDiameter() << sep               // LEFT pupil diameter
        << "FActorName: " << TCHAR_TO_UTF8(*Data.GetFocusActorName()) << sep   // Name (tag) of actor being focused on
        << "FActorPoint: " << VecToString(Data.GetFocusActorPoint()) << sep    // Location of focused actor
        << "FActorDist: " << Data.GetFocusActorDistance() << sep               // Distance to focused actor
        << "VALIDITY: "                                                        // validity booleans
        << "GazeV: " << Data.GetCombinedValidity() << sep                      // validity bool for gaze ray
        << "LGazeV: " << Data.GetLeftValidity() << sep                         // validity for LEFT gaze ray
        << "RGazeV: " << Data.GetRightValidity() << sep                        // validity for RIGHT gaze ray
        << "LEyeOpenV: " << Data.GetLeftEyeOpennessValidity() << sep           // validity for LEFT eye openness
        << "REyeOpenV: " << Data.GetRightEyeOpennessValidity() << sep          // validity for RIGHT eye openness
        << "LPupilPosV: " << Data.GetLeftPupilPositionValidity() << sep        // validity for LEFT pupil position
        << "RPupilPosV: " << Data.GetRightPupilPositionValidity() << sep       // validity for LEFT pupil position
        << "INPUTS: "                                                          // User inputs
        << "Throttle: " << Data.GetUserInputs().Throttle << sep                // Value of Throttle
        << "Steering: " << Data.GetUserInputs().Steering << sep                // Value of Steering
        << "Brake: " << Data.GetUserInputs().Brake << sep                      // Value of brake
        << "ToggleRev: " << Data.GetUserInputs().ToggledReverse << sep         // toggled reverse
        << "TurnSignalLeft: " << Data.GetUserInputs().TurnSignalLeft << sep    // enabled left turn signal
        << "TurnSignalRight: " << Data.GetUserInputs().TurnSignalRight << sep  // enabled right turn signal
        << "Handbrake: " << Data.GetUserInputs().HoldHandbrake << sep          // the handbrake is held
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