#include "Mirror.h"
#include "DReyeVRUtils.h" // ReadConfigValue

AMirror::AMirror(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    static ConstructorHelpers::FObjectFinder<UMaterial> MirrorTexture(
        TEXT("Material'/Game/Carla/Blueprints/Vehicles/DReyeVR/"
             "Mirror_DReyeVR.Mirror_DReyeVR'"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneSM(TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
    // add the static mesh variables
    this->MirrorTexture = MirrorTexture.Object;
    this->PlaneSM = PlaneSM.Object;
    PrimaryActorTick.bCanEverTick = false;
}

void AMirror::ReadConfigVariables()
{
    ReadConfigValue("Mirror", Name + "Enabled", Enabled);
    ReadConfigValue("Mirror", Name + "Location", Location);
    ReadConfigValue("Mirror", Name + "Scale", Scale);
    ReadConfigValue("Mirror", Name + "Rotation", Rotation);
    ReadConfigValue("Mirror", Name + "ReflectionLocation", ReflectionLocation);
    ReadConfigValue("Mirror", Name + "ReflectionScale", ReflectionScale);
    ReadConfigValue("Mirror", Name + "ReflectionRotation", ReflectionRotation);
    ReadConfigValue("Mirror", Name + "ScreenPercentage", ScreenPercentage);
}

void AMirror::Initialize(AWheeledVehicle *Vehicle, const FString &NameIn)
{
    if (!Enabled)
        return;
    Name = NameIn;
    ReadConfigVariables(); // used to read in config params from DReyeVRConfig.ini
    this->AttachToActor(Vehicle, FAttachmentTransformRules::KeepRelativeTransform);
    // Initialize the mirror texture static mesh
    MirrorSM = CreateDefaultSubobject<UStaticMeshComponent>(FName(*(Name + "MirrorSM")));
    MirrorSM->SetStaticMesh(this->PlaneSM);        // set the base mesh to just a rectangle/plane
    MirrorSM->SetMaterial(0, this->MirrorTexture); // use a simple mirror texture on the plane
    MirrorSM->SetupAttachment(Vehicle->GetMesh()); // attach to the vehicle static mesh
    MirrorSM->SetRelativeLocation(Location);
    MirrorSM->SetRelativeRotation(Rotation); // Y Z X (euler angles)
    MirrorSM->SetRelativeScale3D(Scale);
    MirrorSM->SetGenerateOverlapEvents(false); // don't collide with itself
    MirrorSM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MirrorSM->SetVisibility(true);

    // Initialize the planar reflection component
    Reflection = CreateDefaultSubobject<UPlanarReflectionComponent>(FName(*(Name + "Reflection")));
    Reflection->SetupAttachment(MirrorSM); // attach to the mirror static mesh
    Reflection->SetRelativeLocation(ReflectionLocation);
    Reflection->SetRelativeRotation(ReflectionRotation);
    Reflection->SetRelativeScale3D(ReflectionScale);
    Reflection->NormalDistortionStrength = 0.0f;
    Reflection->PrefilterRoughness = 0.0f;
    Reflection->DistanceFromPlaneFadeoutStart = 1500.f;
    Reflection->DistanceFromPlaneFadeoutEnd = 0.f;
    Reflection->AngleFromPlaneFadeStart = 90.f;
    Reflection->AngleFromPlaneFadeEnd = 90.f;
    Reflection->PrefilterRoughnessDistance = 10000.f;
    Reflection->ScreenPercentage = ScreenPercentage; // change this to reduce quality & improve performance
    Reflection->bShowPreviewPlane = false;
    Reflection->HideComponent(Vehicle->GetMesh()); // don't draw the EgoVehicle
    Reflection->SetVisibility(true);
}