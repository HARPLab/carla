// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/CarlaGameInstance.h"

#include "Carla/Settings/CarlaSettings.h"

#include "Carla/Game/LevelScript.h"

UCarlaGameInstance::UCarlaGameInstance() {
  CarlaSettings = CreateDefaultSubobject<UCarlaSettings>(TEXT("CarlaSettings"));
  Recorder = CreateDefaultSubobject<ACarlaRecorder>(TEXT("Recorder"));
  CarlaEngine.SetRecorder(Recorder);

  check(CarlaSettings != nullptr);
  CarlaSettings->LoadSettings();
  CarlaSettings->LogSettings();
}

UCarlaGameInstance::~UCarlaGameInstance() = default;

void UCarlaGameInstance::Init() {
  // Assign the DReyeVR level script
  UEngine* UE4_Engine = GetEngine();
  UE4_Engine->LevelScriptActorClass = ADReyeVRLevel::StaticClass(); 
  UE4_Engine->LevelScriptActorClassName = FSoftClassPath(ADReyeVRLevel::StaticClass());
}