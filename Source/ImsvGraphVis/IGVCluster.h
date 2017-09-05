// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "IGVCluster.generated.h"

USTRUCT()
struct IMSVGRAPHVIS_API FIGVCluster  // Clustering Hierarchy
{
	GENERATED_BODY()

public:
	class AIGVGraphActor* GraphActor;

	UPROPERTY(VisibleAnywhere, SaveGame, Category = ImmersiveGraphVisualization)
	int32 Idx;

	UPROPERTY(VisibleAnywhere, SaveGame, Category = ImmersiveGraphVisualization)
	int32 NodeIdx;  // Leaf clusters are the nodes of a graph

	UPROPERTY(VisibleAnywhere, SaveGame, Category = ImmersiveGraphVisualization)
	int32 ParentIdx;  // Root cluster does not have parent cluster

	UPROPERTY(VisibleAnywhere, SaveGame, Category = ImmersiveGraphVisualization)
	int32 Height;  // Distance from a leaf

	UPROPERTY(VisibleAnywhere, Category = ImmersiveGraphVisualization)
	FVector2D Pos2D;

	UPROPERTY(VisibleAnywhere, Category = ImmersiveGraphVisualization)
	FVector Pos3D;

	FIGVCluster* Parent;
	TArray<FIGVCluster*> Children;
	class AIGVNodeActor* Node;
	int32 NumDescendantNodes;

public:
	FIGVCluster() = default;
	FIGVCluster(class AIGVGraphActor* const InGraphActor);

	FString ToString() const;

	template <class FunctionType>
	void ForEachDescendantFirst(FunctionType Function)
	{
		for (FIGVCluster* const Child : Children)
		{
			Child->ForEachDescendantFirst(Function);
		}
		Function(*this);
	}

	template <class FunctionType>
	void ForEachAncestorFirst(FunctionType Function)
	{
		Function(*this);
		for (FIGVCluster* const Child : Children)
		{
			Child->ForEachAncestorFirst(Function);
		}
	}

	void SetNumDescendantNodes();

	bool IsRoot() const;
	bool IsLeaf() const;

	void SetPos2D(FVector2D const& P);
	void SetPosNonLeaf();

	float DefaultLevel() const;
};
