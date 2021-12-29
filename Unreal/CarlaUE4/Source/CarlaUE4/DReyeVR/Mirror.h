#pragma once
#include "Components/PlanarReflectionComponent.h" // UPlanarReflectionComponent
#include "Components/StaticMeshComponent.h"       // UStaticMeshComponent
#include "CoreMinimal.h"                          // Unreal functions
#include "WheeledVehicle.h"                       // AWheeledVehicle

#include "Mirror.generated.h"

UCLASS()
class AMirror : public AActor
{
    GENERATED_BODY()

  public:
    AMirror(const FObjectInitializer &ObjectInitializer);
    void Initialize(AWheeledVehicle *Vehicle, const FString &NameIn);

  private:
    void ReadConfigVariables();
    UPROPERTY(Category = Mirrors, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    bool Enabled;
    UPROPERTY(Category = Mirrors, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UStaticMeshComponent *MirrorSM;
    UPROPERTY(Category = Mirrors, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UPlanarReflectionComponent *Reflection;
    // UE4 variables
    UMaterial *MirrorTexture;
    UStaticMesh *PlaneSM;
    // 3D attributes
    FVector Location;
    FVector Scale;
    FRotator Rotation;
    // same 3 attributes for reflection
    FVector ReflectionLocation;
    FVector ReflectionScale;
    FRotator ReflectionRotation;
    // other params
    float ScreenPercentage;
    FString Name;
};