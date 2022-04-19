#include "CustomActors.h"
#include "DReyeVRUtils.h" // ReadConfigValue

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

    // parametric material color and lighting is defined in:
    // Material'/Game/DReyeVR/Custom/ParamMaterial.ParamMaterial'
    // and already applied to this static mesh
    AssignSM("StaticMesh'/Game/DReyeVR/Custom/Periph/SM_FixationCross.SM_FixationCross'");

    float EmissionFactor = 0.f;
    ReadConfigValue("PeripheralTarget", "EmissionFactor", EmissionFactor);

    // then you can set the specific parameter values as follows
    // (for more details, see ADReyeVRCustomActor:ApplyMaterialParams)
    ScalarParams = {
        {"Metallic", 1.f},
        {"Specular", 0.f},
        {"Roughness", 1.f},
        {"Anisotropy", 1.f},
    };
    VectorParams = {
        {"BaseColor", FLinearColor::Red},
        {"Emissive", EmissionFactor * FLinearColor::Red},
    };

    // NOTE: this cross has 2 bones (horizontal and vertical components) so we need 2 materials
    NumMaterials = 2; // default is 1 material, change this to apply these params to a group

    // or, you could optionally change the material to something basic such as:
    // AssignMat(0, "MaterialInstanceConstant'/Game/Carla/Static/Vehicles/GeneralMaterials/BrightRed.BrightRed'");
    // AssignMat(1, "MaterialInstanceConstant'/Game/Carla/Static/Vehicles/GeneralMaterials/BrightRed.BrightRed'");

    // finalizing construction
    this->SetActorEnableCollision(false);

    // set internals that are specific to this constructor
    Internals.TypeId = static_cast<char>(DReyeVR::CustomActorData::Types::CROSS);
}

APeriphTarget::APeriphTarget(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    float EmissionFactor = 0.f;
    ReadConfigValue("PeripheralTarget", "EmissionFactor", EmissionFactor);
    bool bUseLegacyPeriphColour = false;
    ReadConfigValue("PeripheralTarget", "UseLegacyPeriphColour", bUseLegacyPeriphColour);

    // parametric material color and lighting is defined in:
    // Material'/Game/DReyeVR/Custom/ParamMaterial.ParamMaterial'
    // and already applied to this static mesh
    AssignSM("StaticMesh'/Game/DReyeVR/Custom/Periph/SM_PeriphTarget.SM_PeriphTarget'");
    // (for more details, see ADReyeVRCustomActor:ApplyMaterialParams)
    if (bUseLegacyPeriphColour)
    {
        // retains anisotropy and specular highlights
        ScalarParams = {};
    }
    else
    {
        // isotropic with no specular highlights
        ScalarParams = {
            {"Metallic", 1.f},
            {"Specular", 0.f},
            {"Roughness", 1.f},
            {"Anisotropy", 1.f},
        };
    }
    VectorParams = {
        {"BaseColor", FLinearColor::Red},
        {"Emissive", EmissionFactor * FLinearColor::Red},
    };
    // finalizing construction
    this->SetActorEnableCollision(false);

    // set internals that are specific to this constructor
    Internals.TypeId = static_cast<char>(DReyeVR::CustomActorData::Types::PERIPH_TARGET);
}
