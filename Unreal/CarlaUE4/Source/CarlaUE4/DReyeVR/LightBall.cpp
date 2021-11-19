// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "LightBall.h"
#include "Components/SphereComponent.h"

// Sets default values
ALightBall::ALightBall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SphereMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Game/LightBallMaterial/Shape_Sphere.Shape_Sphere"));

	if (SphereVisualAsset.Succeeded())
	{
		SphereMesh->SetStaticMesh(SphereVisualAsset.Object);
		//SphereMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	}

	static ConstructorHelpers::FObjectFinder<UMaterialInstance> MaterialAsset(TEXT("/Game/LightBallMaterial/ERed.ERed"));
	MaterialBlue = MaterialAsset.Object;

	SphereMesh->SetMaterial(0, MaterialBlue);

}

// Called when the game starts or when spawned
void ALightBall::BeginPlay()
{
	Super::BeginPlay();

	dynamicMaterial = UMaterialInstanceDynamic::Create(SphereMesh->GetMaterial(0), this);
	//dynamicMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor::Red);
	dynamicMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1,0,0));
	dynamicMaterial->SetScalarParameterValue(TEXT("Emission"), emission);

	SetActorScale3D(FVector(radius, radius, radius));
	SphereMesh->SetMaterial(0, dynamicMaterial);

	// Get Z axis bounds
	FVector Origin;
	FVector Extent;
	GetActorBounds(false, Origin, Extent);
	Extent_Z = FVector(0, 0, Extent.Z);
	
	// start with light being off
	SphereMesh->ToggleVisibility();
	
}

// Called every frame
void ALightBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ALightBall::SetLocation(FVector Posn)
{
	// SphereMesh Origin is set at the bottom of the sphere, so we had to add an offset to center the position
	SetActorLocation(Posn - Extent_Z);
}

void ALightBall::ToggleLight()
{
	SphereMesh->ToggleVisibility();
}

void ALightBall::TurnLightOn()
{
	SphereMesh->SetVisibility(true);
}

void ALightBall::TurnLightOff()
{
	SphereMesh->SetVisibility(false);
}

void ALightBall::SetColor(float R, float G, float B)
{
	dynamicMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(R, G, B));
	SphereMesh->SetMaterial(0, dynamicMaterial);
}

