// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "Core.h"
#include "GameFramework/PlayerController.h"

#include "IGVPlayerController.generated.h"

UCLASS()
class IMSVGRAPHVIS_API AIGVPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = ImmersiveGraphVisualization)
	class AIGVGraphActor* GraphActor;

public:
	AIGVPlayerController();

	virtual void SetupInputComponent() override;

	UFUNCTION(exec)
	void IGV_OpenFile();

	UFUNCTION(exec)
	void IGV_SetFieldOfView(float Value);

	UFUNCTION(exec)
	void IGV_SetAspectRatio(float Value);

	UFUNCTION(exec)
	void IGV_SetTreemapNesting(float Value);

	UFUNCTION(exec)
	void IGV_SetHalo(bool const Value);
};
