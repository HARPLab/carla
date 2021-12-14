// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LightBall.generated.h"

UCLASS()
class CARLAUE4_API ALightBall : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* SphereMesh;
	
public:	
	// Sets default values for this actor's properties
	ALightBall();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInstance* MaterialBlue;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInstanceDynamic* dynamicMaterial;

	float radius = 0.05;
	float emission = 500.f;

	void SetLocation(FVector Posn);
	void ToggleLight();
	void TurnLightOn();
	void TurnLightOff();
	void SetColor(float R, float G, float B);
	void SetSize(double new_radius);

	FVector Extent_Z;
};
