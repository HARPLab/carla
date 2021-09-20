// ========= Copyright 2019, HTC Corporation. All rights reserved. ===========

#include "SRanipal_AvatarEyeSample.h"
#include "SRanipalEye/Public/SRanipalEye_FunctionLibrary.h"
#include "SRanipalEye/Public/SRanipalEye_Framework.h"

#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Public/DrawDebugHelpers.h"
#include "Engine/Classes/Kismet/KismetMathLibrary.h"
#include "EngineUtils.h"
#include "Runtime/Launch/Resources/Version.h"


// Sets default values
ASRanipal_AvatarEyeSample::ASRanipal_AvatarEyeSample()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the avatar mesh components here.
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Movable);
	RootComponent = Root;
	HeadModel = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadModel"));
	HeadModel->SetupAttachment(Root);
	// Create the eye model anchors, the anchors are used as eye model's pivot.
	EyeAnchorLeft = CreateDefaultSubobject<USceneComponent>(TEXT("EyeAnchorLeft"));
	EyeAnchorLeft->SetupAttachment(Root);
	EyeAnchorRight = CreateDefaultSubobject<USceneComponent>(TEXT("EyeAnchorRight"));
	EyeAnchorRight->SetupAttachment(Root);
	// Attach eye models to the anchors.
	EyeModelLeft = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EyeModelLeft"));
	EyeModelLeft->SetupAttachment(EyeAnchorLeft);
	EyeModelRight = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EyeModelRight"));
	EyeModelRight->SetupAttachment(EyeAnchorRight);
	EyeAnchors.Add(EyeAnchorLeft);
	EyeAnchors.Add(EyeAnchorRight);
}

#if WITH_EDITOR
void ASRanipal_AvatarEyeSample::PostEditChangeProperty(struct FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName PropertyName = Event.GetPropertyName();
	// When change model, reset the model's morph target and binding predict weighting.
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASRanipal_AvatarEyeSample, HeadModel)) {
		USkeletalMesh* HeadModelSkelMesh = HeadModel->SkeletalMesh;
		if (HeadModelSkelMesh) {
			TMap<FName, int32> MorphMap = HeadModelSkelMesh->MorphTargetIndexMap;
			for (TPair<FName, int32> Entry : MorphMap) {
				EyeShapeTable.Add(Entry.Key, EyeShape::None);
			}
		}
	}
}
#endif

// Called when the game starts or when spawned
void ASRanipal_AvatarEyeSample::BeginPlay()
{
	Super::BeginPlay();
    SRanipalEye_Framework::Instance()->StartFramework(EyeVersion);
}

void ASRanipal_AvatarEyeSample::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    SRanipalEye_Framework::Instance()->StopFramework();
}

// Called every frame
void ASRanipal_AvatarEyeSample::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateEyeShapes();
	UpdateGazeRay();
}

//int ASRanipal_AvatarEyeSample::GetFrameworkStatus() {
//	ASRanipal_Eye_Framework* framework;
//	for (TActorIterator<ASRanipal_Eye_Framework> it(GetWorld()); it; it++) {
//		framework = *it;
//		break;
//	}
//	return framework->GetStatus();
//}

void ASRanipal_AvatarEyeSample::UpdateEyeShapes()
{
    USRanipalEye_FunctionLibrary::GetEyeWeightings(EyeWeighting);
	RenderModelEyeShape(HeadModel, EyeShapeTable, EyeWeighting);
}

void ASRanipal_AvatarEyeSample::RenderModelEyeShape(USkeletalMeshComponent* Model, TMap<FName, EyeShape> eyeShapeTable, TMap<EyeShape, float> Weighting)
{
	for (TPair<FName, EyeShape> Entry : eyeShapeTable) {
		if (eyeShapeTable[Entry.Key] != EyeShape::None) {
			Model->SetMorphTarget(Entry.Key, Weighting[eyeShapeTable[Entry.Key]]);
		}
	}
}

void ASRanipal_AvatarEyeSample::UpdateGazeRay()
{
	FVector GazeOriginCombinedLocal, GazeDirectionCombinedLocal;
	FVector ModelGazeOrigin, ModelGazeTarget;
	FRotator LookAtRotation, EyeRotator;

	for (int eye = 0; eye < EYE_NUMS; ++eye) {
		bool valid = USRanipalEye_FunctionLibrary::GetGazeRay((GazeIndex)eye, GazeOriginCombinedLocal, GazeDirectionCombinedLocal);
		if (valid) {
#if(ENGINE_MINOR_VERSION>24)
            ModelGazeOrigin = EyeAnchors[eye]->GetRelativeLocation();
            ModelGazeTarget = EyeAnchors[eye]->GetRelativeLocation() + GazeDirectionCombinedLocal;
#else
            ModelGazeOrigin = EyeAnchors[eye]->RelativeLocation;
            ModelGazeTarget = EyeAnchors[eye]->RelativeLocation + GazeDirectionCombinedLocal;
#endif
			LookAtRotation = UKismetMathLibrary::FindLookAtRotation(ModelGazeOrigin, ModelGazeTarget);
			EyeRotator = FRotator(LookAtRotation.Roll, LookAtRotation.Yaw, -LookAtRotation.Pitch);
			EyeAnchors[eye]->SetRelativeRotation(EyeRotator);
		}
	}
}