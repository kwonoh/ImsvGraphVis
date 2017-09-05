// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "Components/TimelineComponent.h"
#include "Core.h"
#include "GameFramework/Actor.h"

#include "IGVNodeActor.generated.h"

UCLASS()
class IMSVGRAPHVIS_API AIGVNodeActor : public AActor
{
	GENERATED_BODY()

public:
	class AIGVGraphActor* GraphActor;

	UPROPERTY(VisibleAnywhere, SaveGame, Category = ImmersiveGraphVisualization)
	int32 Idx;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = ImmersiveGraphVisualization)
	FString Label;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = ImmersiveGraphVisualization)
	FVector2D Pos2D;

	UPROPERTY(BlueprintReadOnly, Category = ImmersiveGraphVisualization)
	FVector Pos3D;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = ImmersiveGraphVisualization)
	TArray<int32> AncIdxs;  // Ancestors in the clustering hierarchy excluding the root

	UPROPERTY(BlueprintReadOnly, Category = ImmersiveGraphVisualization)
	float LevelScale;
	float LevelScaleBeforeTransition;
	float LevelScaleAfterTransition;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = ImmersiveGraphVisualization)
	FLinearColor Color;

	float DistanceToPickRay;

	TArray<struct FIGVEdge*> Edges;
	TArray<AIGVNodeActor*> Neighbors;

	bool bIsHighlighted;
	int32 NumHighlightedNeighbors;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ImmersiveGraphVisualization)
	class UStaticMeshComponent* MeshComponent;

	UPROPERTY(BlueprintReadWrite, Category = ImmersiveGraphVisualization)
	class UMaterialInstanceDynamic* MeshMaterialInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = ImmersiveGraphVisualization)
	class UTextRenderComponent* TextRenderComponent;

public:
	AIGVNodeActor();

	void Init(class AIGVGraphActor* const InGraphActor);

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	FString ToString() const;

	void SetPos3D();

	void SetColor(FLinearColor const& C);
	void SetText(FString const& Value);
	void SetHalo(bool const bValue);

	void UpdateRotation();

	bool IsPicked() const;

	void BeginNearest();
	void EndNearest();

	void BeginPicked();
	void EndPicked();

	void BeginHighlighted();
	void EndHighlighted();

	void BeginNeighborHighlighted();
	void EndNeighborHighlighted();

	bool HasHighlightedNeighbor() const;

	void BeginTransition();

	void OnLeftMouseButtonReleased();

	UFUNCTION(BlueprintImplementableEvent, Category = ImmersiveGraphVisualization)
	void PlayFromStartHighlightTransitionTimeline();

	UFUNCTION(BlueprintImplementableEvent, Category = ImmersiveGraphVisualization)
	void StopHighlightTransitionTimeline();

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	void OnHighlightTransitionTimelineUpdate(ETimelineDirection::Type const Direction,
											 float const Alpha);

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	void OnHighlightTransitionTimelineFinished(ETimelineDirection::Type const Direction);
};
