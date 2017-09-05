// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "IGVPawn.h"

#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Controller.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"

#include "IGVGraphActor.h"
#include "IGVLog.h"

AIGVPawn::AIGVPawn() : CursorDistanceScale(0.4)
{
	PrimaryActorTick.bCanEverTick = true;

	BaseEyeHeight = 0.0f;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->AttachToComponent(RootComponent,
									   FAttachmentTransformRules::KeepRelativeTransform);
	// Disable positional tracking
	CameraComponent->bUsePawnControlRotation = true;
	CameraComponent->bLockToHmd = false;

	CursorMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cursor"));
	CursorMeshComponent->AttachToComponent(CameraComponent,
										   FAttachmentTransformRules::KeepRelativeTransform);
	CursorMeshComponent->SetRelativeLocation(FVector(400, 0, 0));

	CursorDirectionIndicatorMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CursorDirectionIndicator"));
	CursorDirectionIndicatorMeshComponent->AttachToComponent(
		CameraComponent, FAttachmentTransformRules::KeepRelativeTransform);
	CursorDirectionIndicatorMeshComponent->SetRelativeLocation(FVector(200, 0, 0));
	CursorDirectionIndicatorMeshComponent->SetVisibility(false);

	HelpTextRenderComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HelpText"));
	HelpTextRenderComponent->AttachToComponent(CameraComponent,
											   FAttachmentTransformRules::KeepRelativeTransform);
	HelpTextRenderComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	HelpTextRenderComponent->VerticalAlignment = EVerticalTextAligment::EVRTA_TextCenter;
	HelpTextRenderComponent->SetRelativeLocation(FVector(500, 0, 0));
	HelpTextRenderComponent->SetRelativeRotation((-FVector::ForwardVector).Rotation());
	HelpTextRenderComponent->SetVisibility(false);
}

void AIGVPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AIGVPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AIGVPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent->BindAxis(TEXT("MouseX"), this, &AIGVPawn::AddControllerYawInput);
	InputComponent->BindAxis(TEXT("MouseY"), this, &AIGVPawn::AddControllerPitchInput);

	InputComponent->BindAction(TEXT("LeftMouseButton"), IE_Released, this,
							   &AIGVPawn::OnLeftMouseButtonReleased);
}

void AIGVPawn::AddControllerYawInput(float Value)
{
	APlayerController* const PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController)
	{
		PickRayRotation.Yaw += Value * PlayerController->InputYawScale;
		UpdateCursor();
	}
}

void AIGVPawn::AddControllerPitchInput(float Value)
{
	APlayerController* const PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController)
	{
		PickRayRotation.Pitch += -Value * PlayerController->InputPitchScale;
		UpdateCursor();
	}
}

void AIGVPawn::UpdateCursor()
{
	APlayerController* const PlayerController = Cast<APlayerController>(Controller);

	PickRayOrigin = GetActorLocation();
	PickRayDirection = PickRayRotation.Vector().GetSafeNormal();

	FVector const CursorWorldPosition =
		PickRayOrigin + CursorDistanceScale * GraphActor->GetSphereRadius() * PickRayDirection;

	CursorMeshComponent->SetWorldLocation(CursorWorldPosition);
	CursorMeshComponent->SetWorldRotation(PickRayRotation);

	if ((GetViewRotation().Vector() | PickRayDirection) < 0.76604444311 /* cos 40 deg */)
	{
		CursorDirectionIndicatorMeshComponent->SetVisibility(true);
		FVector const RelativeDirection = CameraComponent->GetComponentTransform()
											  .InverseTransformVectorNoScale(PickRayDirection)
											  .GetSafeNormal();
		FVector const TargetDirection =
			FVector(0, RelativeDirection.Y, RelativeDirection.Z).GetSafeNormal();
		CursorDirectionIndicatorMeshComponent->SetRelativeLocation(FVector(200, 0, 0) +
																   TargetDirection * 85);
		CursorDirectionIndicatorMeshComponent->SetRelativeRotation(TargetDirection.Rotation());
	}
	else
	{
		CursorDirectionIndicatorMeshComponent->SetVisibility(false);
	}
}

void AIGVPawn::OnLeftMouseButtonReleased()
{
	if (GraphActor)
	{
		GraphActor->OnLeftMouseButtonReleased();
	}
}
