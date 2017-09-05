// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "IGVCluster.h"
#include "IGVEdge.h"
#include "IGVProjection.h"

#include "IGVGraphActor.generated.h"

UCLASS()
class IMSVGRAPHVIS_API AIGVGraphActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = ImmersiveGraphVisualization)
	FString Filename;

	UPROPERTY(EditDefaultsOnly, Category = ImmersiveGraphVisualization)
	TSubclassOf<class AIGVNodeActor> NodeActorClass;

	UPROPERTY(VisibleAnywhere, Category = ImmersiveGraphVisualization)
	class USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, Category = ImmersiveGraphVisualization)
	class UPostProcessComponent* PostProcessComponent;

	UPROPERTY(VisibleAnywhere, Category = ImmersiveGraphVisualization)
	class USkyLightComponent* SkyLightComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = ImmersiveGraphVisualization)
	class UIGVEdgeMeshComponent* DefaultEdgeGroupMeshComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = ImmersiveGraphVisualization)
	class UIGVEdgeMeshComponent* HighlightedEdgeGroupMeshComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = ImmersiveGraphVisualization)
	class UIGVEdgeMeshComponent* RemainedEdgeGroupMeshComponent;

	UPROPERTY()
	class UMaterialInstanceDynamic* OutlineMaterialInstance;

public:
	TArray<class AIGVNodeActor*> Nodes;
	TArray<FIGVEdge> Edges;
	TArray<FIGVCluster> Clusters;
	FIGVCluster* RootCluster;

	FVector2D PlanarExtent;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame,
			  Category = ImmersiveGraphVisualization,
			  meta = (UIMin = "1.0", UIMax = "360", ClampMin = "1", ClampMax = "360.0"))
	float FieldOfView;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame,
			  Category = ImmersiveGraphVisualization,
			  meta = (UIMin = "0.01", UIMax = "100", ClampMin = "0.01", ClampMax = "100.0"))
	float AspectRatio;  // for Treemap layout

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame,
			  Category = ImmersiveGraphVisualization)
	EIGVProjection ProjectionMode;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame,
			  Category = ImmersiveGraphVisualization)
	float ClusterLevelScale;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame,
			  Category = ImmersiveGraphVisualization)
	float ClusterLevelExponent;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame,
			  Category = ImmersiveGraphVisualization)
	float ClusterLevelOffset;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame,
			  Category = ImmersiveGraphVisualization)
	float TreemapNesting;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame,
			  Category = ImmersiveGraphVisualization)
	int32 EdgeSplineResolution;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame,
			  Category = ImmersiveGraphVisualization)
	float EdgeWidth;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame,
			  Category = ImmersiveGraphVisualization,
			  meta = (ClampMin = "2", ClampMax = "32", UIMin = "2", UIMax = "32"))
	int32 EdgeNumSides;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame,
			  Category = ImmersiveGraphVisualization)
	float EdgeBundlingStrength;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame,
			  Category = ImmersiveGraphVisualization)
	float ColorHueMin;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame,
			  Category = ImmersiveGraphVisualization)
	float ColorHueMax;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame,
			  Category = ImmersiveGraphVisualization)
	float ColorHueOffset;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame,
			  Category = ImmersiveGraphVisualization)
	float ColorChroma;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame,
			  Category = ImmersiveGraphVisualization)
	float ColorLuminance;

	TArray<class AIGVNodeActor*> PickRayDistSortedNodes;
	class AIGVNodeActor* LastNearestNode;
	class AIGVNodeActor* LastPickedNode;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame,
			  Category = ImmersiveGraphVisualization)
	float PickDistanceThreshold;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame, Category = ImmersiveGraph)
	float DefaultLevelScale;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame, Category = ImmersiveGraph)
	float HighlightedLevelScale;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, SaveGame, Category = ImmersiveGraph)
	float NeighborHighlightedLevelScale;

	FGraphEventArray EdgeUpdateTasks;
	bool bUpdateDefaultEdgeMeshRequired;

public:
	AIGVGraphActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	void EmptyGraph();

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	void SetupGraph();

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	float GetSphereRadius() const;

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	void SetSphereRadius(float Radius);

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	FVector Project(FVector2D const& P) const;

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	void UpdatePlanarExtent();

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	void NormalizeNodePosition();

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	void UpdateTreemapLayout();

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	void OnLeftMouseButtonReleased();

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	void SetHalo(bool const bValue);

protected:
	void SetupNodes();
	void SetupEdges();
	void SetupClusters();

	void SetupEdgeMeshes();
	void UpdateEdgeMeshes();

	void UpdateColors();

	void ResetAmbientOcclusion();

	void UpdateInteraction();
	void UpdateNodeDistanceToPickRay();
};
