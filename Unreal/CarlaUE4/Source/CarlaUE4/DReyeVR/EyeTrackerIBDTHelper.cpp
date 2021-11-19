//
// Created by Abhijat on 11/17/2021.
//
#include "EyeTrackerIBDTHelper.h"


float AngleBetweenVectors(const FVector& A, const FVector& B)
{
    float AngleCosine = FVector::DotProduct(A, B) / (A.Size() * B.Size());
    float AngleRadians = FMath::Acos(AngleCosine);
    return FMath::RadiansToDegrees(AngleRadians);
}

std::pair<double, double> Eye2HeadGazeYawPitch (const FVector& eye, const FVector& head)
{

    return std::pair<double, double>
}

GazeDataEntry SensorData2GazeDataEntry(const DReyeVR::SensorData* SensorData)
{
    GazeDataEntry entry;
    double timestamp, x, y, confidence;

    timestamp = SensorData->TimestampCarla;


    // confidence is low if x, y is 0, 0 -- weird bug in VIVE tracke


    return GazeDataEntry(timestamp, confidence, x, y);
}


//void AEyeTrackerIBDT::AddGazePoint(double pitch, double yaw)
//{
//    // GazeDataEntry entry(timestamp, confidence, x, y);
//    GazeDataEntry entry(timestamp, confidence, x, y);
//    ibdt_classifier->addPoint(entry); // entry is passed by reference and its v and classification fields are set
//
//    auto entry_classification = entry.classification;
//    auto entry_velocity = entry.v;
//
//}