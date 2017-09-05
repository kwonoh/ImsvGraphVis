// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

enum class EIGVTreemapOrientation : uint8
{
	Vertical,
	Horizontal
};

struct IMSVGRAPHVIS_API FIGVTreemapNode
{
	struct FIGVCluster* const Cluster;
	TArray<FIGVTreemapNode*> Children;

	FBox2D Rect;
	float Weight;

	FIGVTreemapNode(struct FIGVCluster* const InCluster);

	template <class FunctionType>
	void ForEachDescendantFirst(FunctionType Function)
	{
		for (FIGVTreemapNode* const Child : Children)
		{
			Child->ForEachDescendantFirst(Function);
		}
		Function(*this);
	}

	template <class FunctionType>
	void ForEachAncestorFirst(FunctionType Function)
	{
		Function(*this);
		for (FIGVTreemapNode* const Child : Children)
		{
			Child->ForEachAncestorFirst(Function);
		}
	}

	bool IsLeaf() const;
	void SetRandomWeights();
	void SortChildrenBySize();
};

class IMSVGRAPHVIS_API FIGVTreemapLayout
{
	class AIGVGraphActor* const GraphActor;
	struct FIGVCluster* const RootCluster;

	TArray<FIGVTreemapNode> TreemapNodes;
	FIGVTreemapNode* RootTreemapNode;

public:
	FIGVTreemapLayout(class AIGVGraphActor* const InGraphActor);

	void Compute();

public:
	static void SliceAndDice(FIGVTreemapNode& ParentNode, int32 const FirstIdx, int32 const LastIdx,
							 FBox2D const& Bounds, EIGVTreemapOrientation const Orientation);
	static void SliceAndDice(FIGVTreemapNode& ParentNode, int32 const FirstIdx, int32 const LastIdx,
							 FBox2D const& Bounds);
	static void SliceAndDice(FIGVTreemapNode& ParentNode, FBox2D const& Bounds,
							 EIGVTreemapOrientation const Orientation);
	static void SliceAndDice(FIGVTreemapNode& ParentNode, FBox2D const& Bounds);
	static void SliceAndDice(FIGVTreemapNode& ParentNode, EIGVTreemapOrientation const Orientation);
	static void SliceAndDice(FIGVTreemapNode& ParentNode);

	static void Squarified(FIGVTreemapNode& ParentNode, int32 const FirstIdx, int32 const LastIdx,
						   FBox2D const& Bounds);
	static void Squarified(FIGVTreemapNode& ParentNode, float Nesting);
	static void Squarified(FIGVTreemapNode& ParentNode);

protected:
	void SetupTreemapNodes();

	static double AccumulateWeight(FIGVTreemapNode& ParentNode, int32 const FirstIdx,
								   int32 const LastIdx);
	static float NormalizedAspectRatio(float const Big, float const Small,
									   double const RelativeWeight, double const WeightOffset);
};
