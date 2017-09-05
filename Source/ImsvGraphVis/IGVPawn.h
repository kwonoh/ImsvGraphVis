// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "IGVPawn.generated.h"

UCLASS()
class IMSVGRAPHVIS_API AIGVPawn : public APawn
{
	GENERATED_BODY()

public:
	class AIGVGraphActor* GraphActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ImmersiveGraphVisualization)
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ImmersiveGraphVisualization)
	class UStaticMeshComponent* CursorMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ImmersiveGraphVisualization)
	class UStaticMeshComponent* CursorDirectionIndicatorMeshComponent;

	UPROPERTY(BlueprintReadOnly, Category = ImmersiveGraphVisualization)
	float CursorDistanceScale;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ImmersiveGraphVisualization)
	class UTextRenderComponent* HelpTextRenderComponent;

	UPROPERTY(BlueprintReadOnly, Category = ImmersiveGraphVisualization)
	FRotator PickRayRotation;

	UPROPERTY(BlueprintReadOnly, Category = ImmersiveGraphVisualization)
	FVector PickRayOrigin;

	UPROPERTY(BlueprintReadOnly, Category = ImmersiveGraphVisualization)
	FVector PickRayDirection;

public:
	AIGVPawn();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void AddControllerYawInput(float Value) override;
	virtual void AddControllerPitchInput(float Value) override;

	void UpdateCursor();

	void OnLeftMouseButtonReleased();
};
