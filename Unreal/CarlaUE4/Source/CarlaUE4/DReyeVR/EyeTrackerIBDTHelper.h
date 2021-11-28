//
// Created by Abhijat on 11/17/2021.
//

#include "opencv2/opencv.hpp"
#include "GazeData.h"
#include "IBDT.h"
#include "EgoVehicle.h"
#include "Carla/Sensor/DReyeVRSensor.h"
#include "Carla/Sensor/DReyeVRSensorData.h"

GazeDataEntry SensorData2GazeDataEntry(const DReyeVR::SensorData* SensorData);
float AngleBetweenVectors(const FVector& A, const FVector& B)
