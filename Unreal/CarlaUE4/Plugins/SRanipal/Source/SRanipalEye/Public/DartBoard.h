// ========= Copyright 2019, HTC Corporation. All rights reserved. ===========

#pragma once

#include "SRanipalEye_FunctionLibrary.h"
#include "SRanipalEye.h"

#include "Engine/Classes/Camera/PlayerCameraManager.h"
#include "Runtime/Engine/Classes/Materials/Material.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DartBoard.generated.h"

UCLASS()
class SRANIPALEYE_API ADartBoard : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADartBoard();


	UPROPERTY(EditAnywhere)
		FVector Position;	// The dartboard's position
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* BoardMesh;
	UPROPERTY(EditAnywhere)
		UMaterial* ParentMaterial;

	UPROPERTY()
		APlayerCameraManager* PlayerCameraRef;

private:
	FVector FocusPosition;
	UMaterialInstanceDynamic* DartBoardMaterial;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frames
	virtual void Tick(float DeltaTime) override;

private:
	// Focus relative parameter
	FFocusInfo FocusInfo;
	FVector GazeOrigin, GazeDirection;
	float RayLength = 1000.0f;
	float RayRadius = 1.0f;

	// Move dart board
	FVector InitialPosition;
	bool FirstUpdate = true;
	bool AlreadyMoveDart = false;
	void MoveDartBoard();
	float  SignedAngle(FVector v1, FVector v2, FVector v_forward);
};
