#ifndef DREYEVR_DATA_INL
#define DREYEVR_DATA_INL

#include <string>
#include <unordered_map>

namespace DReyeVR
{

/// ========================================== ///
/// ----------------:EYEDATA:----------------- ///
/// ========================================== ///

inline void EyeData::Read(std::ifstream &InFile)
{
    ReadFVector(InFile, GazeDir);
    ReadFVector(InFile, GazeOrigin);
    ReadValue<bool>(InFile, GazeValid);
}

inline void EyeData::Write(std::ofstream &OutFile) const
{
    WriteFVector(OutFile, GazeDir);
    WriteFVector(OutFile, GazeOrigin);
    WriteValue<bool>(OutFile, GazeValid);
}

inline FString EyeData::ToString() const
{
    FString Print;
    Print += FString::Printf(TEXT("GazeDir:%s,"), *GazeDir.ToString());
    Print += FString::Printf(TEXT("GazeOrigin:%s,"), *GazeOrigin.ToString());
    Print += FString::Printf(TEXT("GazeValid:%d,"), GazeValid);
    return Print;
}

/// ========================================== ///
/// ------------:COMBINEDEYEDATA:------------- ///
/// ========================================== ///

inline void CombinedEyeData::Read(std::ifstream &InFile)
{
    EyeData::Read(InFile);
    ReadValue<float>(InFile, Vergence);
}

inline void CombinedEyeData::Write(std::ofstream &OutFile) const
{
    EyeData::Write(OutFile);
    WriteValue<float>(OutFile, Vergence);
}

inline FString CombinedEyeData::ToString() const
{
    FString Print = EyeData::ToString();
    Print += FString::Printf(TEXT("GazeVergence:%.4f,"), Vergence);
    return Print;
}

/// ========================================== ///
/// -------------:SINGLEEYEDATA:-------------- ///
/// ========================================== ///

inline void SingleEyeData::Read(std::ifstream &InFile)
{
    EyeData::Read(InFile);
    ReadValue<float>(InFile, EyeOpenness);
    ReadValue<bool>(InFile, EyeOpennessValid);
    ReadValue<float>(InFile, PupilDiameter);
    ReadFVector2D(InFile, PupilPosition);
    ReadValue<bool>(InFile, PupilPositionValid);
}

inline void SingleEyeData::Write(std::ofstream &OutFile) const
{
    EyeData::Write(OutFile);
    WriteValue<float>(OutFile, EyeOpenness);
    WriteValue<bool>(OutFile, EyeOpennessValid);
    WriteValue<float>(OutFile, PupilDiameter);
    WriteFVector2D(OutFile, PupilPosition);
    WriteValue<bool>(OutFile, PupilPositionValid);
}

inline FString SingleEyeData::ToString() const
{
    FString Print = EyeData::ToString();
    Print += FString::Printf(TEXT("EyeOpenness:%.4f,"), EyeOpenness);
    Print += FString::Printf(TEXT("EyeOpennessValid:%d,"), EyeOpennessValid);
    Print += FString::Printf(TEXT("PupilDiameter:%.4f,"), PupilDiameter);
    Print += FString::Printf(TEXT("PupilPosition:%s,"), *PupilPosition.ToString());
    Print += FString::Printf(TEXT("PupilPositionValid:%d,"), PupilPositionValid);
    return Print;
}

/// ========================================== ///
/// --------------:EGOVARIABLES:-------------- ///
/// ========================================== ///

inline void EgoVariables::Read(std::ifstream &InFile)
{
    ReadFVector(InFile, CameraLocation);
    ReadFRotator(InFile, CameraRotation);
    ReadFVector(InFile, CameraLocationAbs);
    ReadFRotator(InFile, CameraRotationAbs);
    ReadFVector(InFile, VehicleLocation);
    ReadFRotator(InFile, VehicleRotation);
    ReadValue<float>(InFile, Velocity);
}

inline void EgoVariables::Write(std::ofstream &OutFile) const
{
    WriteFVector(OutFile, CameraLocation);
    WriteFRotator(OutFile, CameraRotation);
    WriteFVector(OutFile, CameraLocationAbs);
    WriteFRotator(OutFile, CameraRotationAbs);
    WriteFVector(OutFile, VehicleLocation);
    WriteFRotator(OutFile, VehicleRotation);
    WriteValue<float>(OutFile, Velocity);
}

inline FString EgoVariables::ToString() const
{
    FString Print;
    Print += FString::Printf(TEXT("VehicleLoc:%s,"), *VehicleLocation.ToString());
    Print += FString::Printf(TEXT("VehicleRot:%s,"), *VehicleRotation.ToString());
    Print += FString::Printf(TEXT("VehicleVel:%.4f,"), Velocity);
    Print += FString::Printf(TEXT("CameraLoc:%s,"), *CameraLocation.ToString());
    Print += FString::Printf(TEXT("CameraRot:%s,"), *CameraRotation.ToString());
    Print += FString::Printf(TEXT("CameraLocAbs:%s,"), *CameraLocationAbs.ToString());
    Print += FString::Printf(TEXT("CameraRotAbs:%s,"), *CameraRotationAbs.ToString());
    return Print;
}

/// ========================================== ///
/// --------------:USERINPUTS:---------------- ///
/// ========================================== ///

inline void UserInputs::Read(std::ifstream &InFile)
{
    ReadValue<float>(InFile, Throttle);
    ReadValue<float>(InFile, Steering);
    ReadValue<float>(InFile, Brake);
    ReadValue<bool>(InFile, ToggledReverse);
    ReadValue<bool>(InFile, TurnSignalLeft);
    ReadValue<bool>(InFile, TurnSignalRight);
    ReadValue<bool>(InFile, HoldHandbrake);
}

inline void UserInputs::Write(std::ofstream &OutFile) const
{
    WriteValue<float>(OutFile, Throttle);
    WriteValue<float>(OutFile, Steering);
    WriteValue<float>(OutFile, Brake);
    WriteValue<bool>(OutFile, ToggledReverse);
    WriteValue<bool>(OutFile, TurnSignalLeft);
    WriteValue<bool>(OutFile, TurnSignalRight);
    WriteValue<bool>(OutFile, HoldHandbrake);
}

inline FString UserInputs::ToString() const
{
    FString Print;
    Print += FString::Printf(TEXT("Throttle:%.4f,"), Throttle);
    Print += FString::Printf(TEXT("Steering:%.4f,"), Steering);
    Print += FString::Printf(TEXT("Brake:%.4f,"), Brake);
    Print += FString::Printf(TEXT("ToggledReverse:%d,"), ToggledReverse);
    Print += FString::Printf(TEXT("TurnSignalLeft:%d,"), TurnSignalLeft);
    Print += FString::Printf(TEXT("TurnSignalRight:%d,"), TurnSignalRight);
    Print += FString::Printf(TEXT("HoldHandbrake:%d,"), HoldHandbrake);
    return Print;
}

/// ========================================== ///
/// ----------------:LEGACY:------------------ ///
/// ========================================== ///

inline void LegacyPeriphDataStruct::Read(std::ifstream &InFile)
{
    ReadFVector(InFile, WorldPos);
    ReadFRotator(InFile, WorldRot);
    ReadFVector(InFile, CombinedOrigin);
    ReadValue<float>(InFile, gaze2target_pitch);
    ReadValue<float>(InFile, gaze2target_yaw);
    ReadValue<float>(InFile, head2target_pitch);
    ReadValue<float>(InFile, head2target_yaw);
    ReadValue<bool>(InFile, Visible);
    ReadValue<bool>(InFile, TriggerPressed);
}

inline FString LegacyPeriphDataStruct::ToString() const
{
    FString Print;
    Print += FString::Printf(TEXT("WorldPos:%s,"), *WorldPos.ToString());
    Print += FString::Printf(TEXT("WorldRot:%s,"), *WorldRot.ToString());
    Print += FString::Printf(TEXT("CombinedOrigin:%s,"), *CombinedOrigin.ToString());
    Print += FString::Printf(TEXT("gaze2target_pitch:%.4f,"), gaze2target_pitch);
    Print += FString::Printf(TEXT("gaze2target_yaw:%.4f,"), gaze2target_yaw);
    Print += FString::Printf(TEXT("head2target_pitch:%.4f,"), head2target_pitch);
    Print += FString::Printf(TEXT("head2target_yaw:%.4f,"), head2target_yaw);
    Print += FString::Printf(TEXT("LightOn:%d,"), Visible);
    Print += FString::Printf(TEXT("ButtonPressed:%d,"), TriggerPressed);
    return Print;
}

const inline LegacyPeriphDataStruct &AggregateData::GetLegacyPeriphData() const
{
    return LegacyPeriphData;
}

/// ========================================== ///
/// ---------------:FOCUSINFO:---------------- ///
/// ========================================== ///

inline void FocusInfo::Read(std::ifstream &InFile)
{
    ReadFString(InFile, ActorNameTag);
    ReadValue<bool>(InFile, bDidHit);
    ReadFVector(InFile, HitPoint);
    ReadFVector(InFile, Normal);
    ReadValue<float>(InFile, Distance);
}

inline void FocusInfo::Write(std::ofstream &OutFile) const
{
    WriteFString(OutFile, ActorNameTag);
    WriteValue<bool>(OutFile, bDidHit);
    WriteFVector(OutFile, HitPoint);
    WriteFVector(OutFile, Normal);
    WriteValue<float>(OutFile, Distance);
}

inline FString FocusInfo::ToString() const
{
    FString Print;
    Print += FString::Printf(TEXT("Hit:%d,"), bDidHit);
    Print += FString::Printf(TEXT("Distance:%.4f,"), Distance);
    Print += FString::Printf(TEXT("HitPoint:%s,"), *HitPoint.ToString());
    Print += FString::Printf(TEXT("HitNormal:%s,"), *Normal.ToString());
    Print += FString::Printf(TEXT("ActorName:%s,"), *ActorNameTag);
    return Print;
}

/// ========================================== ///
/// ---------------:EYETRACKER:--------------- ///
/// ========================================== ///

inline void EyeTracker::Read(std::ifstream &InFile)
{
    ReadValue<int64_t>(InFile, TimestampDevice);
    ReadValue<int64_t>(InFile, FrameSequence);
    Combined.Read(InFile);
    Left.Read(InFile);
    Right.Read(InFile);
}

inline void EyeTracker::Write(std::ofstream &OutFile) const
{
    WriteValue<int64_t>(OutFile, TimestampDevice);
    WriteValue<int64_t>(OutFile, FrameSequence);
    Combined.Write(OutFile);
    Left.Write(OutFile);
    Right.Write(OutFile);
}

inline FString EyeTracker::ToString() const
{
    FString Print;
    Print += FString::Printf(TEXT("TimestampDevice:%ld,"), long(TimestampDevice));
    Print += FString::Printf(TEXT("FrameSequence:%ld,"), long(FrameSequence));
    Print += FString::Printf(TEXT("COMBINED:{%s},"), *Combined.ToString());
    Print += FString::Printf(TEXT("LEFT:{%s},"), *Left.ToString());
    Print += FString::Printf(TEXT("RIGHT:{%s},"), *Right.ToString());
    return Print;
}

/// ========================================== ///
/// -------------:AGGREGATEDATA:-------------- ///
/// ========================================== ///

inline int64_t AggregateData::GetTimestampCarla() const
{
    return TimestampCarlaUE4;
}

inline int64_t AggregateData::GetTimestampDevice() const
{
    return EyeTrackerData.TimestampDevice;
}

inline int64_t AggregateData::GetFrameSequence() const
{
    return EyeTrackerData.FrameSequence;
}

inline float AggregateData::GetGazeVergence() const
{
    return EyeTrackerData.Combined.Vergence; // in cm (default UE4 units)
}

inline const FVector &AggregateData::GetGazeDir(DReyeVR::Gaze Index) const
{
    switch (Index)
    {
    case DReyeVR::Gaze::LEFT:
        return EyeTrackerData.Left.GazeDir;
    case DReyeVR::Gaze::RIGHT:
        return EyeTrackerData.Right.GazeDir;
    case DReyeVR::Gaze::COMBINED:
        return EyeTrackerData.Combined.GazeDir;
    default: // need a default case for MSVC >:(
        return EyeTrackerData.Combined.GazeDir;
    }
}

inline const FVector &AggregateData::GetGazeOrigin(DReyeVR::Gaze Index) const
{
    switch (Index)
    {
    case DReyeVR::Gaze::LEFT:
        return EyeTrackerData.Left.GazeOrigin;
    case DReyeVR::Gaze::RIGHT:
        return EyeTrackerData.Right.GazeOrigin;
    case DReyeVR::Gaze::COMBINED:
        return EyeTrackerData.Combined.GazeOrigin;
    default: // need a default case for MSVC >:(
        return EyeTrackerData.Combined.GazeOrigin;
    }
}

inline bool AggregateData::GetGazeValidity(DReyeVR::Gaze Index) const
{
    switch (Index)
    {
    case DReyeVR::Gaze::LEFT:
        return EyeTrackerData.Left.GazeValid;
    case DReyeVR::Gaze::RIGHT:
        return EyeTrackerData.Right.GazeValid;
    case DReyeVR::Gaze::COMBINED:
        return EyeTrackerData.Combined.GazeValid;
    default: // need a default case for MSVC >:(
        return EyeTrackerData.Combined.GazeValid;
    }
}

inline float AggregateData::GetEyeOpenness(DReyeVR::Eye Index) const // returns eye openness as a percentage [0,1]
{
    switch (Index)
    {
    case DReyeVR::Eye::LEFT:
        return EyeTrackerData.Left.EyeOpenness;
    case DReyeVR::Eye::RIGHT:
        return EyeTrackerData.Right.EyeOpenness;
    default: // need a default case for MSVC >:(
        return EyeTrackerData.Right.EyeOpenness;
    }
}

inline bool AggregateData::GetEyeOpennessValidity(DReyeVR::Eye Index) const
{
    switch (Index)
    {
    case DReyeVR::Eye::LEFT:
        return EyeTrackerData.Left.EyeOpennessValid;
    case DReyeVR::Eye::RIGHT:
        return EyeTrackerData.Right.EyeOpennessValid;
    default: // need a default case for MSVC >:(
        return EyeTrackerData.Right.EyeOpennessValid;
    }
}

inline float AggregateData::GetPupilDiameter(DReyeVR::Eye Index) const // returns diameter in mm
{
    switch (Index)
    {
    case DReyeVR::Eye::LEFT:
        return EyeTrackerData.Left.PupilDiameter;
    case DReyeVR::Eye::RIGHT:
        return EyeTrackerData.Right.PupilDiameter;
    default: // need a default case for MSVC >:(
        return EyeTrackerData.Right.PupilDiameter;
    }
}

inline const FVector2D &AggregateData::GetPupilPosition(DReyeVR::Eye Index) const
{
    switch (Index)
    {
    case DReyeVR::Eye::LEFT:
        return EyeTrackerData.Left.PupilPosition;
    case DReyeVR::Eye::RIGHT:
        return EyeTrackerData.Right.PupilPosition;
    default: // need a default case for MSVC >:(
        return EyeTrackerData.Right.PupilPosition;
    }
}

inline bool AggregateData::GetPupilPositionValidity(DReyeVR::Eye Index) const
{
    switch (Index)
    {
    case DReyeVR::Eye::LEFT:
        return EyeTrackerData.Left.PupilPositionValid;
    case DReyeVR::Eye::RIGHT:
        return EyeTrackerData.Right.PupilPositionValid;
    default: // need a default case for MSVC >:(
        return EyeTrackerData.Right.PupilPositionValid;
    }
}

// from EgoVars
inline const FVector &AggregateData::GetCameraLocation() const
{
    return EgoVars.CameraLocation;
}

inline const FRotator &AggregateData::GetCameraRotation() const
{
    return EgoVars.CameraRotation;
}

inline const FVector &AggregateData::GetCameraLocationAbs() const
{
    return EgoVars.CameraLocationAbs;
}

inline const FRotator &AggregateData::GetCameraRotationAbs() const
{
    return EgoVars.CameraRotationAbs;
}

inline float AggregateData::GetVehicleVelocity() const
{
    return EgoVars.Velocity; // returns ego velocity in cm/s
}

inline const FVector &AggregateData::GetVehicleLocation() const
{
    return EgoVars.VehicleLocation;
}

inline const FRotator &AggregateData::GetVehicleRotation() const
{
    return EgoVars.VehicleRotation;
}

// focus
inline const FString &AggregateData::GetFocusActorName() const
{
    return FocusData.ActorNameTag;
}

inline const FVector &AggregateData::GetFocusActorPoint() const
{
    return FocusData.HitPoint;
}

inline float AggregateData::GetFocusActorDistance() const
{
    return FocusData.Distance;
}

inline const DReyeVR::UserInputs &AggregateData::GetUserInputs() const
{
    return Inputs;
}

inline void AggregateData::UpdateCamera(const FVector &NewCameraLoc, const FRotator &NewCameraRot)
{
    EgoVars.CameraLocation = NewCameraLoc;
    EgoVars.CameraRotation = NewCameraRot;
}

inline void AggregateData::UpdateCameraAbs(const FVector &NewCameraLocAbs, const FRotator &NewCameraRotAbs)
{
    EgoVars.CameraLocationAbs = NewCameraLocAbs;
    EgoVars.CameraRotationAbs = NewCameraRotAbs;
}

inline void AggregateData::UpdateVehicle(const FVector &NewVehicleLoc, const FRotator &NewVehicleRot)
{
    EgoVars.VehicleLocation = NewVehicleLoc;
    EgoVars.VehicleRotation = NewVehicleRot;
}

inline void AggregateData::Update(int64_t NewTimestamp, const struct EyeTracker &NewEyeData,
                                  const struct EgoVariables &NewEgoVars, const struct FocusInfo &NewFocus,
                                  const struct UserInputs &NewInputs)
{
    TimestampCarlaUE4 = NewTimestamp;
    EyeTrackerData = NewEyeData;
    EgoVars = NewEgoVars;
    FocusData = NewFocus;
    Inputs = NewInputs;
}

inline void AggregateData::Read(std::ifstream &InFile)
{
    /// CAUTION: make sure the order of writes/reads is the same
    ReadValue<int64_t>(InFile, TimestampCarlaUE4);
    EgoVars.Read(InFile);
    EyeTrackerData.Read(InFile);
    FocusData.Read(InFile);
    Inputs.Read(InFile);
    if (bUsingLegacyPeriphFile)
        LegacyPeriphData.Read(InFile);
}

inline void AggregateData::Write(std::ofstream &OutFile) const
{
    /// CAUTION: make sure the order of writes/reads is the same
    WriteValue<int64_t>(OutFile, GetTimestampCarla());
    EgoVars.Write(OutFile);
    EyeTrackerData.Write(OutFile);
    FocusData.Write(OutFile);
    Inputs.Write(OutFile);
}

inline FString AggregateData::ToString() const
{
    FString print;
    print += FString::Printf(TEXT("  [DReyeVR]TimestampCarla:%ld,\n"), long(TimestampCarlaUE4));
    print += FString::Printf(TEXT("  [DReyeVR]EyeTracker:%s,\n"), *EyeTrackerData.ToString());
    print += FString::Printf(TEXT("  [DReyeVR]FocusInfo:%s,\n"), *FocusData.ToString());
    print += FString::Printf(TEXT("  [DReyeVR]EgoVariables:%s,\n"), *EgoVars.ToString());
    print += FString::Printf(TEXT("  [DReyeVR]UserInputs:%s,"), *Inputs.ToString());
    if (bUsingLegacyPeriphFile)
        print += LegacyPeriphData.ToString();
    print += "\n";
    return print;
}

inline std::string AggregateData::GetUniqueName() const
{
    return "DReyeVRSensorAggregateData";
}

/// ========================================== ///
/// ------------:CUSTOMACTORDATA:------------- ///
/// ========================================== ///

inline void CustomActorData::Read(std::ifstream &InFile)
{
    ReadValue<char>(InFile, TypeId);
    ReadFVector(InFile, Location);
    ReadFRotator(InFile, Rotation);
    ReadFVector(InFile, Scale3D);
    ReadFString(InFile, Other);
    ReadFString(InFile, Name);
}

inline void CustomActorData::Write(std::ofstream &OutFile) const
{
    WriteValue<char>(OutFile, static_cast<char>(TypeId));
    WriteFVector(OutFile, Location);
    WriteFRotator(OutFile, Rotation);
    WriteFVector(OutFile, Scale3D);
    WriteFString(OutFile, Other);
    WriteFString(OutFile, Name);
}

inline FString CustomActorData::ToString() const
{
    FString Print = "  [DReyeVR_CA]";
    Print += FString::Printf(TEXT("Type:%d,"), int(TypeId));
    Print += FString::Printf(TEXT("Name:%s,"), *Name);
    Print += FString::Printf(TEXT("Location:%s,"), *Location.ToString());
    Print += FString::Printf(TEXT("Rotation:%s,"), *Rotation.ToString());
    Print += FString::Printf(TEXT("Scale3D:%s,"), *Scale3D.ToString());
    Print += FString::Printf(TEXT("Other:%s,"), *Other);
    return Print;
}

inline std::string CustomActorData::GetUniqueName() const
{
    return TCHAR_TO_UTF8(*Name);
}

}; // namespace DReyeVR

#endif