// ========= Copyright 2018, HTC Corporation. All rights reserved. ===========

#pragma once

#include "SRanipalLip_Core.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SRanipal_AvatarLipSample.generated.h"


/**
* This is the sample actor to make an avatar with skeletal mesh.
* In this sample, demonstrate how to use SRanipal's Lip functions to achieve face expression animate.
*/
UCLASS()
class SRANIPALLIP_API ASRanipal_AvatarLipSample : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASRanipal_AvatarLipSample();

    // Declare Lip version
    UPROPERTY(EditAnywhere)
        SupportedLipVersion LipVersion;

	// Declare your avatar component here.
	UPROPERTY(EditAnywhere)
        USceneComponent *Root;
	// The mesh components.
	UPROPERTY(EditAnywhere)
		USkeletalMeshComponent *HeadModel;
	UPROPERTY(EditAnywhere)
		USkeletalMeshComponent *EyeLeft;
	UPROPERTY(EditAnywhere)
		USkeletalMeshComponent *EyeRight;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

    TMap<FName, LipShapeEnum> LipShapeTable;
    TMap<FName, LipShapeEnum_v2> LipShapeTable_v2;

	// Used TMap to save return weightings.
    TMap<LipShapeEnum, float> LipWeighting;
	TMap<LipShapeEnum_v2, float> LipWeighting_v2;

	void UpdateLipShapes();
    template<typename T>
	void RenderModelLipShape(USkeletalMeshComponent* Model, TMap<FName, T> lipShapeTable, TMap<T, float> Weighting);
};
