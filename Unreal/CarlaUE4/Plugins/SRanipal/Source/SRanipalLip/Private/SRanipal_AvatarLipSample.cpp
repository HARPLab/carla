// ========= Copyright 2018, HTC Corporation. All rights reserved. ===========

#include "SRanipal_AvatarLipSample.h"
#include "SRanipalLip.h"
#include "SRanipalLip_FunctionLibrary.h"
#include "SRanipalLip/Public/SRanipalLip_Framework.h"

#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Public/DrawDebugHelpers.h"
#include "Engine/Classes/Kismet/KismetMathLibrary.h"
#include "EngineUtils.h"

// Sets default values
ASRanipal_AvatarLipSample::ASRanipal_AvatarLipSample()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

	// Create the avatar mesh components here.
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Movable);
	RootComponent = Root;
	HeadModel = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadModel"));
	HeadModel->SetupAttachment(Root);
	EyeLeft = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftEyeModel"));
	EyeLeft->SetupAttachment(Root);
	EyeRight = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightEyeModel"));
	EyeRight->SetupAttachment(Root);
}



// Called when the game starts or when spawned
void ASRanipal_AvatarLipSample::BeginPlay()
{
	Super::BeginPlay();
    SRanipalLip_Framework::Instance()->StartFramework(LipVersion);
    switch (LipVersion) {
    case SupportedLipVersion::version1:
        LipShapeTable = { 
        {"Jaw_Forward", LipShapeEnum::Jaw_Forward},
        {"Jaw_Right", LipShapeEnum::Jaw_Right},
        {"Jaw_Left", LipShapeEnum::Jaw_Left},
        {"Jaw_Open", LipShapeEnum::Jaw_Open},
        {"Mouth_Ape_Shape", LipShapeEnum::Mouth_Ape_Shape},
        {"Mouth_O_Shape", LipShapeEnum::Mouth_O_Shape},
        {"Mouth_Pout", LipShapeEnum::Mouth_Pout},
        {"Mouth_Lower_Right", LipShapeEnum::Mouth_Lower_Right},
        {"Mouth_Lower_Left", LipShapeEnum::Mouth_Lower_Left},
        {"Mouth_Smile_Right", LipShapeEnum::Mouth_Smile_Right},
        {"Mouth_Smile_Left", LipShapeEnum::Mouth_Smile_Left},
        {"Mouth_Sad_Right", LipShapeEnum::Mouth_Sad_Right},
        {"Mouth_Sad_Left", LipShapeEnum::Mouth_Sad_Left},
        {"Cheek_Puff_Right", LipShapeEnum::Cheek_Puff_Right},
        {"Cheek_Puff_Left", LipShapeEnum::Cheek_Puff_Left},
        {"Mouth_Lower_Inside", LipShapeEnum::Mouth_Lower_Inside},
        {"Mouth_Upper_Inside", LipShapeEnum::Mouth_Upper_Inside},
        {"Mouth_Lower_Overlay", LipShapeEnum::Mouth_Lower_Overlay},
        {"Mouth_Upper_Overlay", LipShapeEnum::Mouth_Upper_Overlay},
        {"Cheek_Suck", LipShapeEnum::Cheek_Suck},
        {"Mouth_LowerRight_Down", LipShapeEnum::Mouth_LowerRight_Down},
        {"Mouth_LowerLeft_Down", LipShapeEnum::Mouth_LowerLeft_Down},
        {"Mouth_UpperRight_Up", LipShapeEnum::Mouth_UpperRight_Up},
        {"Mouth_UpperLeft_Up", LipShapeEnum::Mouth_UpperLeft_Up},
        {"Mouth_Philtrum_Right", LipShapeEnum::Mouth_Philtrum_Right},
        {"Mouth_Philtrum_Left", LipShapeEnum::Mouth_Philtrum_Left} };
        break;
    case SupportedLipVersion::version2:
        LipShapeTable_v2 = { 
           {"Jaw_Right", LipShapeEnum_v2::Jaw_Right},
           {"Jaw_Left", LipShapeEnum_v2::Jaw_Left},
           {"Jaw_Forward", LipShapeEnum_v2::Jaw_Forward},
           {"Jaw_Open", LipShapeEnum_v2::Jaw_Open},
           {"Mouth_Ape_Shape", LipShapeEnum_v2::Mouth_Ape_Shape},
           {"Mouth_Upper_Right", LipShapeEnum_v2::Mouth_Upper_Right},
           {"Mouth_Upper_Left", LipShapeEnum_v2::Mouth_Upper_Left},
           {"Mouth_Lower_Right", LipShapeEnum_v2::Mouth_Lower_Right},
           {"Mouth_Lower_Left", LipShapeEnum_v2::Mouth_Lower_Left},
           {"Mouth_Upper_Overturn", LipShapeEnum_v2::Mouth_Upper_Overturn},
           {"Mouth_Lower_Overturn", LipShapeEnum_v2::Mouth_Lower_Overturn},
           {"Mouth_Pout", LipShapeEnum_v2::Mouth_Pout},
           {"Mouth_Smile_Right", LipShapeEnum_v2::Mouth_Smile_Right},
           {"Mouth_Smile_Left", LipShapeEnum_v2::Mouth_Smile_Left},
           {"Mouth_Sad_Right", LipShapeEnum_v2::Mouth_Sad_Right},
           {"Mouth_Sad_Left", LipShapeEnum_v2::Mouth_Sad_Left},
           {"Cheek_Puff_Right", LipShapeEnum_v2::Cheek_Puff_Right},
           {"Cheek_Puff_Left", LipShapeEnum_v2::Cheek_Puff_Left},
           {"Cheek_Suck", LipShapeEnum_v2::Cheek_Suck},
           {"Mouth_Upper_UpRight", LipShapeEnum_v2::Mouth_Upper_UpRight},
           {"Mouth_Upper_UpLeft", LipShapeEnum_v2::Mouth_Upper_UpLeft},
           {"Mouth_Lower_DownRight", LipShapeEnum_v2::Mouth_Lower_DownRight},
           {"Mouth_Lower_DownLeft", LipShapeEnum_v2::Mouth_Lower_DownLeft},
           {"Mouth_Upper_Inside", LipShapeEnum_v2::Mouth_Upper_Inside},
           {"Mouth_Lower_Inside", LipShapeEnum_v2::Mouth_Lower_Inside},
           {"Mouth_Lower_Overlay", LipShapeEnum_v2::Mouth_Lower_Overlay},
           {"Tongue_LongStep1", LipShapeEnum_v2::Tongue_LongStep1},
           {"Tongue_Left", LipShapeEnum_v2::Tongue_Left},
           {"Tongue_Right", LipShapeEnum_v2::Tongue_Right},
           {"Tongue_Up", LipShapeEnum_v2::Tongue_Up},
           {"Tongue_Down", LipShapeEnum_v2::Tongue_Down},
           {"Tongue_Roll", LipShapeEnum_v2::Tongue_Roll},
           {"Tongue_LongStep2", LipShapeEnum_v2::Tongue_LongStep2},
           {"Tongue_UpRight_Morph", LipShapeEnum_v2::Tongue_UpRight_Morph},
           {"Tongue_UpLeft_Morph", LipShapeEnum_v2::Tongue_UpLeft_Morph},
           {"Tongue_DownRight_Morph", LipShapeEnum_v2::Tongue_DownRight_Morph},
           {"Tongue_DownLeft_Morph", LipShapeEnum_v2::Tongue_DownLeft_Morph} };
        break;
    default:
        break;
    }
    SetActorTickEnabled(true);
}

// Called every frame
void ASRanipal_AvatarLipSample::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateLipShapes();
}

void ASRanipal_AvatarLipSample::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    SRanipalLip_Framework::Instance()->StopFramework();
    LipShapeTable.Empty();
    LipShapeTable_v2.Empty();
}

void ASRanipal_AvatarLipSample::UpdateLipShapes()
{
    switch (LipVersion) {
    case SupportedLipVersion::version1:
            USRanipalLip_FunctionLibrary::GetLipWeightings(LipWeighting);
            if (LipWeighting.Num() == (int)LipShapeEnum::Max) RenderModelLipShape(HeadModel, LipShapeTable, LipWeighting);
            break;
    case SupportedLipVersion::version2:
            USRanipalLip_FunctionLibrary::GetLipWeightings_v2(LipWeighting_v2);
            if (LipWeighting_v2.Num() == (int)LipShapeEnum_v2::Max)RenderModelLipShape(HeadModel, LipShapeTable_v2, LipWeighting_v2);
            break;
    default:
        break;
    }

}

template<typename T>
void ASRanipal_AvatarLipSample::RenderModelLipShape(USkeletalMeshComponent* Model, TMap<FName, T> lipShapeTable, TMap<T, float> Weighting)
{
    for (auto &table : lipShapeTable) {
		if ((int)table.Value != (int)LipShapeEnum_v2::None) {
			Model->SetMorphTarget(table.Key, Weighting[table.Value]);
		}
	}
}