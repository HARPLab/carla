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
    Data.Read(InFile);
}

/// ========================================== ///
/// ---------------:WRITERS:------------------ ///
/// ========================================== ///

void DReyeVRDataRecorder::Write(std::ofstream &OutFile) const
{
    Data.Write(OutFile);
}

/// ========================================== ///
/// ---------------:PRINTING:----------------- ///
/// ========================================== ///

std::string DReyeVRDataRecorder::Print() const
{
    std::ostringstream oss;
    oss << Data.ToString();
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