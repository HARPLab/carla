// ========= Copyright 2019, HTC Corporation. All rights reserved. ===========

#include "DartBoard.h"
#include "SRanipal/Public/Eye/SRanipal_API_Eye.h"

#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Engine/Classes/Kismet/GameplayStatics.h"
#include "Engine/Classes/Kismet/KismetMathLibrary.h"
#include "math.h"
#include "Engine/World.h"


// Sets default values
ADartBoard::ADartBoard()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create dartboard's mesh
	BoardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DartBoardMesh"));
	RootComponent = BoardMesh;
	BoardMesh->SetMobility(EComponentMobility::Movable);
}

// Called when the game starts or when spawned
void ADartBoard::BeginPlay()
{
	Super::BeginPlay();

	// Make material instances and set this material to mesh
	DartBoardMaterial = UMaterialInstanceDynamic::Create(ParentMaterial, this);
	BoardMesh->SetMaterial(0, DartBoardMaterial);

	// Find main camera reference
	PlayerCameraRef = UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager;
	if (PlayerCameraRef == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Cant find player's camera manager."));
	}

	// Record dart board relative position.
	InitialPosition = GetActorLocation();
}

// Called every frame
void ADartBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!AlreadyMoveDart) {
		MoveDartBoard();
	}
	bool hit = USRanipalEye_FunctionLibrary::Focus(GazeIndex::COMBINE, RayLength, RayRadius, PlayerCameraRef, ECollisionChannel::ECC_WorldStatic, FocusInfo, GazeOrigin, GazeDirection);
	FocusPosition = FocusInfo.point;

	unsigned int bendCount = 7;
	unsigned int pieCount = 8;
	unsigned int frameWidth = 1;

	float maxDist = 125.0f * GetActorScale().X;
	float sectionLength = maxDist / bendCount;
	float dist = FVector::Distance(FocusPosition, GetActorLocation());
	unsigned int bendIndex = (unsigned int)(dist / sectionLength);

	FVector axis = (FocusPosition - GetActorLocation()) / FVector::Distance(FocusPosition, GetActorLocation());
	float ang = SignedAngle(GetActorRightVector(), axis, GetActorForwardVector());
	if (ang < 0) ang += 360.0f;
	float pieDegree = 360.0f / pieCount;
	unsigned int pieIndex = (unsigned int)(ang / pieDegree);

	DartBoardMaterial->SetVectorParameterValue("MeshCenter", GetActorLocation());
	DartBoardMaterial->SetVectorParameterValue("MeshForward", GetActorForwardVector());
	DartBoardMaterial->SetVectorParameterValue("MeshRight", GetActorRightVector());
	DartBoardMaterial->SetScalarParameterValue("BendCount", bendCount);
	DartBoardMaterial->SetScalarParameterValue("PieCount", pieCount);
	DartBoardMaterial->SetScalarParameterValue("BendIndex", bendIndex);
	DartBoardMaterial->SetScalarParameterValue("PieIndex", pieIndex);
	DartBoardMaterial->SetScalarParameterValue("Scale", GetActorScale().Y);
	DartBoardMaterial->SetScalarParameterValue("FrameWidth", frameWidth);
}

float ADartBoard::SignedAngle(FVector v1, FVector v2, FVector v_forward)
{
	float dotP = FVector::DotProduct(v1, v2);
	float unsignedAngle = acos(dotP) * (180 / 3.14159f);

	float sign = FVector::DotProduct(v_forward, FVector::CrossProduct(v1, v2));
	float signedAngle = unsignedAngle * (sign > 0.0f ? 1.0f : -1.0f) + 180.0f;
	return signedAngle;
}

void ADartBoard::MoveDartBoard() {
    float openness;
    bool EyeValid = USRanipalEye_FunctionLibrary::GetEyeOpenness(EyeIndex::LEFT, openness);
	if (EyeValid && !FirstUpdate) {
		// Move DartBoard to relative position
		FVector TargetPlace;
		FVector CameraLocation = PlayerCameraRef->GetCameraLocation();
		TargetPlace = CameraLocation + InitialPosition;
		BoardMesh->SetWorldLocation(TargetPlace);

		// Rotate the DartBoard and let it face to user.
		FRotator DartBoardRotator = UKismetMathLibrary::FindLookAtRotation(TargetPlace, PlayerCameraRef->GetCameraLocation());
		DartBoardRotator.Yaw += 180.f;
		DartBoardRotator.Pitch = 0.f;
		BoardMesh->SetWorldRotation(DartBoardRotator);

		AlreadyMoveDart = true;
	}
	FirstUpdate = false;
}