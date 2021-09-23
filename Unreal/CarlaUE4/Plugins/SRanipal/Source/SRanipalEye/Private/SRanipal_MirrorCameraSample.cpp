// ========= Copyright 2019, HTC Corporation. All rights reserved. ===========

#include "SRanipal_MirrorCameraSample.h"
#include "SRanipalEye.h"
#include "SRanipalEye_FunctionLibrary.h"

#include "Engine.h"
#include "Engine/Classes/Kismet/KismetMathLibrary.h"


// Sets default values
ASRanipal_MirrorCameraSample::ASRanipal_MirrorCameraSample()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Movable);
	RootComponent = Root;
}

// Called when the game starts or when spawned
void ASRanipal_MirrorCameraSample::BeginPlay()
{
	Super::BeginPlay();

	// Get player's CameraManager
	PlayerCameraRef = UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager;
	if (PlayerCameraRef == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Cant find player's camera manager."));
	}
}

// Called every frame
void ASRanipal_MirrorCameraSample::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector origin, direction;
    FVector2D pupil_left;

    // Move mirror in front of the camera.
    FVector TargetPlace;
    FVector CameraLocation = PlayerCameraRef->GetCameraLocation();
    FRotator CameraRotation = PlayerCameraRef->GetCameraRotation();
    TargetPlace = CameraRotation.Vector() * Distance + CameraLocation;
    Root->SetWorldLocation(TargetPlace);

    // Rotate the mirror and let it face to user.
    FRotator MirrorRotator = UKismetMathLibrary::FindLookAtRotation(TargetPlace, PlayerCameraRef->GetCameraLocation());
    MirrorRotator.Yaw += 180.f;
    MirrorRotator.Pitch = 0.f;
    Root->SetWorldRotation(MirrorRotator);
    if (USRanipalEye_FunctionLibrary::GetGazeRay(GazeIndex::COMBINE, origin, direction)) {
        // Disable the mirror actor's Tick() so the mirror wont be moved again.
        SetActorTickEnabled(false);
    }
}
