#include "DReyeVRCustomActor.h"
#include "Carla/Game/CarlaStatics.h"    // GetEpisode
#include "Carla/Sensor/DReyeVRSensor.h" // ADReyeVRSensor::bIsReplaying
#include "Materials/MaterialInstance.h" // UMaterialInstance
#include "UObject/ConstructorHelpers.h" // ConstructorHelpers

#include <string>

std::unordered_map<std::string, class ADReyeVRCustomActor *> ADReyeVRCustomActor::ActiveCustomActors = {};
int ADReyeVRCustomActor::AllMeshCount = 0;

ADReyeVRCustomActor::ADReyeVRCustomActor(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    // done in child class
    Internals.Location = this->GetActorLocation();
    Internals.Rotation = this->GetActorRotation();
    Internals.Scale3D = this->GetActorScale3D();
}

void ADReyeVRCustomActor::AssignSM(const FString &Path)
{
    FName MeshName(*("Mesh" + FString::FromInt(ADReyeVRCustomActor::AllMeshCount)));
    ActorMesh = CreateDefaultSubobject<UStaticMeshComponent>(MeshName);
    ActorMesh->SetupAttachment(this->GetRootComponent());
    this->SetRootComponent(ActorMesh);

    ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(*Path);
    if (MeshAsset.Succeeded())
        ActorMesh->SetStaticMesh(MeshAsset.Object);
    else
        UE_LOG(LogTemp, Error, TEXT("Unable to access mesh asset: %s"), *Path)
    // static count for how many meshes have been processed thus far
    ADReyeVRCustomActor::AllMeshCount++;
}

void ADReyeVRCustomActor::AssignMat(const int MatIdx, const FString &Path)
{
    ConstructorHelpers::FObjectFinder<UMaterialInstance> MaterialAsset(*Path);
    if (MaterialAsset.Succeeded() && ActorMesh != nullptr)
        ActorMesh->SetMaterial(MatIdx, CastChecked<UMaterialInterface>(MaterialAsset.Object));
    else
        UE_LOG(LogTemp, Error, TEXT("Unable to access material asset: %s"), *Path)
}

void ADReyeVRCustomActor::Initialize(const FString &Name)
{
    Internals.Name = Name;
    ADReyeVRCustomActor::ActiveCustomActors[TCHAR_TO_UTF8(*Name)] = this;
    UE_LOG(LogTemp, Log, TEXT("Initialized custom actor: %s"), *Name);
}

void ADReyeVRCustomActor::BeginPlay()
{
    Super::BeginPlay();
}

void ADReyeVRCustomActor::BeginDestroy()
{
    Super::BeginDestroy();
}

void ADReyeVRCustomActor::Disable()
{
    const std::string s = TCHAR_TO_UTF8(*Internals.Name);
    // UE_LOG(LogTemp, Log, TEXT("Disabling custom actor: %s"), *Internals.Name);
    if (ADReyeVRCustomActor::ActiveCustomActors.find(s) != ADReyeVRCustomActor::ActiveCustomActors.end())
    {
        ADReyeVRCustomActor::ActiveCustomActors.erase(s);
    }
    this->SetActorHiddenInGame(true);
    this->SetActorTickEnabled(false);
    this->bIsEnabled = false;
}

void ADReyeVRCustomActor::Enable()
{
    // UE_LOG(LogTemp, Log, TEXT("Enabling custom actor: %s"), *Internals.Name);
    ADReyeVRCustomActor::ActiveCustomActors[TCHAR_TO_UTF8(*Internals.Name)] = this;
    this->SetActorHiddenInGame(false);
    this->SetActorTickEnabled(true);
    this->bIsEnabled = true;
}

void ADReyeVRCustomActor::Tick(float DeltaSeconds)
{
    if (ADReyeVRSensor::bIsReplaying)
    {
        // update world state with internals
        this->SetActorLocation(Internals.Location);
        this->SetActorRotation(Internals.Rotation);
        this->SetActorScale3D(Internals.Scale3D);
    }
    else
    {
        // update internals with world state
        Internals.Location = this->GetActorLocation();
        Internals.Rotation = this->GetActorRotation();
        Internals.Scale3D = this->GetActorScale3D();
    }
    /// TODO: use other string?
}

void ADReyeVRCustomActor::SetInternals(const DReyeVR::CustomActorData &InData)
{
    Internals = InData;
}

const DReyeVR::CustomActorData &ADReyeVRCustomActor::GetInternals() const
{
    return Internals;
}