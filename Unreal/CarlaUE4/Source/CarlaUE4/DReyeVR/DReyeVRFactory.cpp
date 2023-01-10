#include "DReyeVRFactory.h"
#include "Carla.h"                                     // to avoid linker errors
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h" // UActorBlueprintFunctionLibrary
#include "Carla/Actor/VehicleParameters.h"             // FVehicleParameters
#include "Carla/Game/CarlaEpisode.h"                   // UCarlaEpisode
#include "EgoVehicle.h"                                // AEgoVehicle

#define EgoVehicleBP_Str "/Game/Carla/Blueprints/Vehicles/DReyeVR/BP_EgoVehicle_DReyeVR.BP_EgoVehicle_DReyeVR_C"

ADReyeVRFactory::ADReyeVRFactory(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    // get ego vehicle bp (can use UTF8_TO_TCHAR if making EgoVehicleBP_Str a variable)
    static ConstructorHelpers::FObjectFinder<UClass> EgoVehicleBP(TEXT(EgoVehicleBP_Str));
    EgoVehicleBPClass = EgoVehicleBP.Object;
    ensure(EgoVehicleBPClass != nullptr);
}

TArray<FActorDefinition> ADReyeVRFactory::GetDefinitions()
{
    FVehicleParameters Parameters;
    Parameters.Make = "DReyeVR";
    Parameters.Model = "Model3";
    Parameters.ObjectType = EgoVehicleBP_Str;
    Parameters.Class = AEgoVehicle::StaticClass();

    // need to create an FActorDefinition from our FActorDescription for some reason -_-
    FActorDefinition Definition;
    bool Success = false;
    UActorBlueprintFunctionLibrary::MakeVehicleDefinition(Parameters, Success, Definition);
    if (!Success)
    {
        LOG_ERROR("Unable to create DReyeVR vehicle definition!");
    }
    Definition.Class = Parameters.Class;
    return {Definition};
}

FActorSpawnResult ADReyeVRFactory::SpawnActor(const FTransform &SpawnAtTransform,
                                              const FActorDescription &ActorDescription)
{
    auto *World = GetWorld();
    if (World == nullptr)
    {
        LOG_ERROR("cannot spawn \"%s\" into an empty world.", *ActorDescription.Id);
        return {};
    }

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    LOG("Spawning EgoVehicle (\"%s\") at: %s", *ActorDescription.Id, *SpawnAtTransform.ToString());
    auto *EgoVehicleActor = World->SpawnActor<AEgoVehicle>(EgoVehicleBPClass, SpawnAtTransform, SpawnParameters);
    return FActorSpawnResult(EgoVehicleActor);
}
