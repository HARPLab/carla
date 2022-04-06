#include "CustomActors.h"

ABall::ABall(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    AssignSM("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'");

    AssignMat(0, "MaterialInstanceConstant'/Game/Carla/Static/Vehicles/GeneralMaterials/BrightRed.BrightRed'");

    // finalizing construction
    this->SetActorEnableCollision(false);

    // set internals that are specific to this constructor
    Internals.TypeId = static_cast<char>(DReyeVR::CustomActorData::Types::SPHERE);
}

ACross::ACross(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    // material color and lighting is defined in: Material'/Game/DReyeVR/Custom/Periph/EmissiveRed.EmissiveRed'
    AssignSM("StaticMesh'/Game/DReyeVR/Custom/Periph/SMFixationCross.SMFixationCross'");

    // finalizing construction
    this->SetActorEnableCollision(false);

    // set internals that are specific to this constructor
    Internals.TypeId = static_cast<char>(DReyeVR::CustomActorData::Types::CROSS);
}

APeriphTarget::APeriphTarget(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    // material color and lighting is defined in: Material'/Game/DReyeVR/Custom/Periph/EmissiveRed.EmissiveRed'
    AssignSM("StaticMesh'/Game/DReyeVR/Custom/Periph/SMPeriphTarget.SMPeriphTarget'");

    // finalizing construction
    this->SetActorEnableCollision(false);

    // set internals that are specific to this constructor
    Internals.TypeId = static_cast<char>(DReyeVR::CustomActorData::Types::PERIPH_TARGET);
}
