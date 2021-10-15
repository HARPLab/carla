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

void ReadTimings(std::ifstream &InFile, DReyeVR::SensorData &Snapshot)
{
    // all the timing values in SRanipal
    ReadValue<int64_t>(InFile, Snapshot.TimestampSR);
    ReadValue<int64_t>(InFile, Snapshot.TimestampCarla);
    ReadValue<int64_t>(InFile, Snapshot.FrameSequence);
}

void ReadEyeData(std::ifstream &InFile, DReyeVR::EyeData &Eye)
{
    ReadFVector(InFile, Eye.GazeRay);
    ReadFVector(InFile, Eye.Origin);
    ReadValue<bool>(InFile, Eye.GazeValid);
}

void ReadComboEyeData(std::ifstream &InFile, DReyeVR::ComboEyeData &Eye)
{
    // specifically for the combined eye gaze data
    ReadEyeData(InFile, Eye);
    ReadValue<float>(InFile, Eye.Vergence);
}

void ReadSingleEyeData(std::ifstream &InFile, DReyeVR::SingleEyeData &Eye)
{
    // specifically for the left/right eye gaze data
    ReadEyeData(InFile, Eye);
    ReadValue<float>(InFile, Eye.EyeOpenness);
    ReadValue<bool>(InFile, Eye.EyeOpenValid);
    ReadValue<float>(InFile, Eye.PupilDiam);
    ReadFVector2D(InFile, Eye.PupilPos);
    ReadValue<bool>(InFile, Eye.PupilPosValid);
}

void ReadFocusActor(std::ifstream &InFile, DReyeVR::SensorData &Snapshot)
{
    ReadFString(InFile, Snapshot.FocusActorName);
    ReadFVector(InFile, Snapshot.FocusActorPoint);
    ReadValue<float>(InFile, Snapshot.FocusActorDist);
}

void ReadInputs(std::ifstream &InFile, DReyeVR::UserInputs &In)
{
    ReadValue<float>(InFile, In.Throttle);
    ReadValue<float>(InFile, In.Steering);
    ReadValue<float>(InFile, In.Brake);
    ReadValue<bool>(InFile, In.ToggledReverse);
    ReadValue<bool>(InFile, In.TurnSignalLeft);
    ReadValue<bool>(InFile, In.TurnSignalRight);
    ReadValue<bool>(InFile, In.HoldHandbrake);
}

void DReyeVRDataRecorder::Read(std::ifstream &InFile)
{
    ReadTimings(InFile, this->Data);
    ReadComboEyeData(InFile, this->Data.Combined);
    ReadSingleEyeData(InFile, this->Data.Left);
    ReadSingleEyeData(InFile, this->Data.Right);
    ReadFVector(InFile, this->Data.HMDLocation);
    ReadFRotator(InFile, this->Data.HMDRotation);
    ReadFocusActor(InFile, this->Data);
    ReadInputs(InFile, this->Data.Inputs);
    ReadValue<float>(InFile, this->Data.Velocity);
}

/// ========================================== ///
/// ---------------:WRITERS:------------------ ///
/// ========================================== ///

void WriteTimings(std::ofstream &OutFile, const DReyeVR::SensorData &Snapshot)
{
    // all the timing values in SRanipal
    WriteValue<int64_t>(OutFile, Snapshot.TimestampSR);
    WriteValue<int64_t>(OutFile, Snapshot.TimestampCarla);
    WriteValue<int64_t>(OutFile, Snapshot.FrameSequence);
}

void WriteEyeData(std::ofstream &OutFile, const DReyeVR::EyeData &Eye)
{
    WriteFVector(OutFile, Eye.GazeRay);
    WriteFVector(OutFile, Eye.Origin);
    WriteValue<bool>(OutFile, Eye.GazeValid);
}

void WriteComboEyeData(std::ofstream &OutFile, const DReyeVR::ComboEyeData &Eye)
{
    // specifically for the combined eye gaze data
    WriteEyeData(OutFile, Eye);
    WriteValue<float>(OutFile, Eye.Vergence);
}

void WriteSingleEyeData(std::ofstream &OutFile, const DReyeVR::SingleEyeData &Eye)
{
    // specifically for the left/right eye gaze data
    WriteEyeData(OutFile, Eye);
    WriteValue<float>(OutFile, Eye.EyeOpenness);
    WriteValue<bool>(OutFile, Eye.EyeOpenValid);
    WriteValue<float>(OutFile, Eye.PupilDiam);
    WriteFVector2D(OutFile, Eye.PupilPos);
    WriteValue<bool>(OutFile, Eye.PupilPosValid);
}

void WriteFocusActor(std::ofstream &OutFile, const DReyeVR::SensorData &Snapshot)
{
    WriteFString(OutFile, Snapshot.FocusActorName);
    WriteFVector(OutFile, Snapshot.FocusActorPoint);
    WriteValue<float>(OutFile, Snapshot.FocusActorDist);
}

void WriteInputs(std::ofstream &OutFile, const DReyeVR::UserInputs &In)
{
    WriteValue<float>(OutFile, In.Throttle);
    WriteValue<float>(OutFile, In.Steering);
    WriteValue<float>(OutFile, In.Brake);
    WriteValue<bool>(OutFile, In.ToggledReverse);
    WriteValue<bool>(OutFile, In.TurnSignalLeft);
    WriteValue<bool>(OutFile, In.TurnSignalRight);
    WriteValue<bool>(OutFile, In.HoldHandbrake);
}

void DReyeVRDataRecorder::Write(std::ofstream &OutFile) const
{
    WriteTimings(OutFile, this->Data);
    WriteComboEyeData(OutFile, this->Data.Combined);
    WriteSingleEyeData(OutFile, this->Data.Left);
    WriteSingleEyeData(OutFile, this->Data.Right);
    WriteFVector(OutFile, this->Data.HMDLocation);
    WriteFRotator(OutFile, this->Data.HMDRotation);
    WriteFocusActor(OutFile, this->Data);
    WriteInputs(OutFile, this->Data.Inputs);
    WriteValue<float>(OutFile, this->Data.Velocity);
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
    const std::string delim = "; ";                                       // semicolon + space
    oss << "T_SRanipal: " << Data.TimestampSR << delim                    // Sranipal time
        << "T_Carla: " << Data.TimestampCarla << delim                    // Carla time
        << "FrameSeq: " << Data.FrameSequence << delim                    // SRanipal Framesequence
        << "GazeRay: " << VecToString(Data.Combined.GazeRay) << delim     // Gaze ray vector
        << "EyeOrigin: " << VecToString(Data.Combined.Origin) << delim    // Combined Eye origin vector
        << "Vergence: " << Data.Combined.Vergence << delim                // Calculated vergence
        << "HMDLoc: " << VecToString(Data.HMDLocation) << delim           // HMD location
        << "HMDRot: " << VecToString(Data.HMDRotation) << delim           // HMD rotation
        << "EgoVel: " << Data.Velocity << delim                           // Ego Velocity
        << "LGazeRay: " << VecToString(Data.Left.GazeRay) << delim        // LEFT gaze ray
        << "LEyeOrigin: " << VecToString(Data.Left.Origin) << delim       // LEFT gaze ray
        << "RGazeRay: " << VecToString(Data.Right.GazeRay) << delim       // RIGHT gaze ray
        << "REyeOrigin: " << VecToString(Data.Right.Origin) << delim      // RIGHT gaze ray
        << "LEyeOpenness: " << Data.Left.EyeOpenness << delim             // LEFT eye openness
        << "REyeOpenness: " << Data.Right.EyeOpenness << delim            // RIGHT eye openness
        << "LPupilPos: " << VecToString(Data.Left.PupilPos) << delim      // LEFT pupil position
        << "RPupilPos: " << VecToString(Data.Right.PupilPos) << delim     // RIGHT pupil position
        << "LPupilDiam: " << Data.Left.PupilDiam << delim                 // LEFT pupil diameter
        << "RPupilDiam: " << Data.Right.PupilDiam << delim                // LEFT pupil diameter
        << "FActorName: " << TCHAR_TO_UTF8(*Data.FocusActorName) << delim // Name (tag) of actor being focused on
        << "FActorPoint: " << VecToString(Data.FocusActorPoint) << delim  // Location of focused actor
        << "FActorDist: " << Data.FocusActorDist << delim                 // Distance to focused actor
        << "VALIDITY: "                                                   // validity booleans
        << "GazeV: " << Data.Combined.GazeValid << delim                  // validity bool for gaze ray
        << "LGazeV: " << Data.Left.GazeValid << delim                     // validity for LEFT gaze ray
        << "RGazeV: " << Data.Right.GazeValid << delim                    // validity for RIGHT gaze ray
        << "LEyeOpenV: " << Data.Left.EyeOpenValid << delim               // validity for LEFT eye openness
        << "REyeOpenV: " << Data.Right.EyeOpenValid << delim              // validity for RIGHT eye openness
        << "LPupilPosV: " << Data.Left.PupilPosValid << delim             // validity for LEFT pupil position
        << "RPupilPosV: " << Data.Right.PupilPosValid << delim            // validity for LEFT pupil position
        << "INPUTS: "                                                     // User inputs
        << "Throttle: " << Data.Inputs.Throttle << delim                  // Value of Throttle
        << "Steering: " << Data.Inputs.Steering << delim                  // Value of Steering
        << "Brake: " << Data.Inputs.Brake << delim                        // Value of brake
        << "ToggleRev: " << Data.Inputs.ToggledReverse << delim           // Whether or not toggled reverse
        << "TurnSignalLeft: " << Data.Inputs.TurnSignalLeft << delim      // Whether or not enabled left turn signal
        << "TurnSignalRight: " << Data.Inputs.TurnSignalRight << delim    // Whether or not enabled right turn signal
        << "Handbrake: " << Data.Inputs.HoldHandbrake << delim            // Whether or not the handbrake is held
        ;
    return oss.str();
}
//---------------------------------------------

void DReyeVRDataRecorders::Clear(void)
{
    AllSnapshots.clear();
}

void DReyeVRDataRecorders::Add(const DReyeVRDataRecorder &NewData)
{
    AllSnapshots.push_back(NewData);
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
    Total = AllSnapshots.size();
    WriteValue<uint16_t>(OutFile, Total);

    for (auto &Snapshot : AllSnapshots)
        Snapshot.Write(OutFile);

    /// TODO: check if we need this? or can just write Total * sizeof(DReyeVRData)

    // write the real packet size
    std::streampos PosEnd = OutFile.tellp();
    Total = PosEnd - PosStart - sizeof(uint32_t);
    OutFile.seekp(PosStart, std::ios::beg);
    WriteValue<uint32_t>(OutFile, Total);
    OutFile.seekp(PosEnd, std::ios::beg);
}