// ========= Copyright 2019, HTC Corporation. All rights reserved. ===========

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Classes/Camera/PlayerCameraManager.h"
#include "SRanipal_MirrorCameraSample.generated.h"


/**
* This is the sample actor can move a 2D mirror in front of user.
* The actor can let user see avatar's facial expression in mirror.
*/
UCLASS()
class SRANIPALEYE_API ASRanipal_MirrorCameraSample : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASRanipal_MirrorCameraSample();

	UPROPERTY()
		USceneComponent* Root;
	UPROPERTY()
		APlayerCameraManager* PlayerCameraRef;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	bool FirstFrame = true;
	const float Distance = 50.f;
};
