// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.
// A port of treemap implementation by HCIL http://www.cs.umd.edu/hcil/treemap/

#include "IGVTreemapLayout.h"

#include "IGVGraphActor.h"
#include "IGVLog.h"

FIGVTreemapNode::FIGVTreemapNode(FIGVCluster* const InCluster)
	: Cluster(InCluster), Children(), Rect(), Weight(1.f)
{
}

bool FIGVTreemapNode::IsLeaf() const
{
	return Children.Num() == 0;
}

void FIGVTreemapNode::SetRandomWeights()
{
	ForEachDescendantFirst([](FIGVTreemapNode& Node) {
		if (Node.IsLeaf())
		{
			Node.Weight = FMath::FRandRange(1.f, 2.f);
		}
		else
		{
			for (FIGVTreemapNode* const Child : Node.Children)
			{
				Node.Weight += Child->Weight;
			}
		}
	});
}

void FIGVTreemapNode::SortChildrenBySize()
{
	Children.Sort(
		[](FIGVTreemapNode const& A, FIGVTreemapNode const& B) { return A.Weight > B.Weight; });
}

FIGVTreemapLayout::FIGVTreemapLayout(AIGVGraphActor* const InGraphActor)
	: GraphActor(InGraphActor),
	  RootCluster(GraphActor->RootCluster),
	  TreemapNodes(),
	  RootTreemapNode(nullptr)
{
}

void FIGVTreemapLayout::Compute()
{
	SetupTreemapNodes();

	RootTreemapNode->SetRandomWeights();
	RootTreemapNode->Rect = FBox2D(-GraphActor->PlanarExtent, GraphActor->PlanarExtent);

	float const Nesting = GraphActor->TreemapNesting;
	RootTreemapNode->ForEachAncestorFirst([Nesting](FIGVTreemapNode& Node) {
		if (!Node.IsLeaf())
		{
			Squarified(Node, Nesting);
		}
	});

	for (FIGVTreemapNode& TreemapNode : TreemapNodes)
	{
		FVector2D Center, Extents;
		TreemapNode.Rect.GetCenterAndExtents(Center, Extents);
		Center.Y *= -1;
		FVector2D const Offset = 0.5 * FVector2D(FMath::FRandRange(-Extents.X, Extents.X),
												 FMath::FRandRange(-Extents.Y, Extents.Y));
		TreemapNode.Cluster->SetPos2D(Center + Offset);
	}
}

void FIGVTreemapLayout::SetupTreemapNodes()
{
	for (FIGVCluster& Cluster : GraphActor->Clusters)
	{
		TreemapNodes.Emplace(&Cluster);
	}

	for (FIGVCluster& Cluster : GraphActor->Clusters)
	{
		if (!Cluster.IsRoot())
		{
			TreemapNodes[Cluster.ParentIdx].Children.Add(&TreemapNodes[Cluster.Idx]);
		}
	}

	RootTreemapNode = &TreemapNodes.Last();
}

void FIGVTreemapLayout::SliceAndDice(FIGVTreemapNode& ParentNode, int32 const FirstIdx,
									 int32 const LastIdx, FBox2D const& Bounds,
									 EIGVTreemapOrientation const Orientation)
{
	if (LastIdx == FirstIdx)
	{
		ParentNode.Children[FirstIdx]->Rect = Bounds;
		return;
	}

	double const AccumWeight = AccumulateWeight(ParentNode, FirstIdx, LastIdx);
	FVector2D const BoundsSize = Bounds.GetSize();

	double RelativeWeightOffset = 0;

	if (Orientation == EIGVTreemapOrientation::Vertical)
	{
		for (int32 Idx = FirstIdx; Idx <= LastIdx; Idx++)
		{
			FIGVTreemapNode* Child = ParentNode.Children[Idx];
			FBox2D& Rect = Child->Rect;
			double const RelativeWeight = Child->Weight / AccumWeight;

			Rect.Min.X = Bounds.Min.X;
			Rect.Max.X = Bounds.Max.X;
			Rect.Min.Y = Bounds.Min.Y + BoundsSize.Y * RelativeWeightOffset;
			Rect.Max.Y = Rect.Min.Y + BoundsSize.Y * RelativeWeight;

			RelativeWeightOffset += RelativeWeight;
		}
	}
	else
	{
		for (int32 Idx = FirstIdx; Idx <= LastIdx; Idx++)
		{
			FIGVTreemapNode* Child = ParentNode.Children[Idx];
			FBox2D& Rect = Child->Rect;
			double const RelativeWeight = Child->Weight / AccumWeight;

			Rect.Min.Y = Bounds.Min.Y;
			Rect.Max.Y = Bounds.Max.Y;
			Rect.Min.X = Bounds.Min.X + BoundsSize.X * RelativeWeightOffset;
			Rect.Max.X = Rect.Min.X + BoundsSize.X * RelativeWeight;

			RelativeWeightOffset += RelativeWeight;
		}
	}
}

void FIGVTreemapLayout::SliceAndDice(FIGVTreemapNode& ParentNode, int32 const FirstIdx,
									 int32 const LastIdx, FBox2D const& Bounds)
{
	FVector2D const BoundsSize = Bounds.GetSize();
	SliceAndDice(ParentNode, FirstIdx, LastIdx, Bounds,
				 BoundsSize.X > BoundsSize.Y ? EIGVTreemapOrientation::Horizontal
											 : EIGVTreemapOrientation::Vertical);
}

void FIGVTreemapLayout::SliceAndDice(FIGVTreemapNode& ParentNode, const FBox2D& Bounds,
									 const EIGVTreemapOrientation Orientation)
{
	SliceAndDice(ParentNode, 0, ParentNode.Children.Num() - 1, Bounds, Orientation);
}

void FIGVTreemapLayout::SliceAndDice(FIGVTreemapNode& ParentNode, const FBox2D& Bounds)
{
	SliceAndDice(ParentNode, 0, ParentNode.Children.Num() - 1, Bounds);
}

void FIGVTreemapLayout::SliceAndDice(FIGVTreemapNode& ParentNode,
									 EIGVTreemapOrientation const Orientation)
{
	SliceAndDice(ParentNode, 0, ParentNode.Children.Num() - 1, ParentNode.Rect, Orientation);
}

void FIGVTreemapLayout::SliceAndDice(FIGVTreemapNode& ParentNode)
{
	SliceAndDice(ParentNode, 0, ParentNode.Children.Num() - 1, ParentNode.Rect);
}

void FIGVTreemapLayout::Squarified(FIGVTreemapNode& ParentNode, int32 const FirstIdx,
								   int32 const LastIdx, FBox2D const& Bounds)
{
	if (FirstIdx > LastIdx)
	{
		return;
	}

	if (LastIdx - FirstIdx < 2)
	{
		return SliceAndDice(ParentNode, FirstIdx, LastIdx, Bounds);
	}

	double const AccumWeight = AccumulateWeight(ParentNode, FirstIdx, LastIdx);

	int32 MidIdx = FirstIdx;
	double const FirstRelativeWeight = ParentNode.Children[FirstIdx]->Weight / AccumWeight;
	double RelativeWeightOffset = FirstRelativeWeight;

	FVector2D const BoundsSize = Bounds.GetSize();
	float const X = Bounds.Min.X;
	float const Y = Bounds.Min.Y;
	float const W = BoundsSize.X;
	float const H = BoundsSize.Y;

	if (W < H)
	{
		while (MidIdx <= LastIdx)
		{
			float const AspectRatio =
				NormalizedAspectRatio(H, W, FirstRelativeWeight, RelativeWeightOffset);
			float const RelativeWeight = ParentNode.Children[MidIdx]->Weight / AccumWeight;

			if (NormalizedAspectRatio(H, W, FirstRelativeWeight,
									  RelativeWeightOffset + RelativeWeight) > AspectRatio)
				break;

			MidIdx++;
			RelativeWeightOffset += RelativeWeight;
		}

		SliceAndDice(ParentNode, FirstIdx, MidIdx,
					 FBox2D(FVector2D(X, Y), FVector2D(X + W, Y + H * RelativeWeightOffset)));

		FVector2D const NextCornerMin(X, Y + H * RelativeWeightOffset);

		return Squarified(
			ParentNode, MidIdx + 1, LastIdx,
			FBox2D(NextCornerMin, NextCornerMin + FVector2D(W, H * (1 - RelativeWeightOffset))));
	}
	else
	{
		while (MidIdx <= LastIdx)
		{
			float const AspectRatio =
				NormalizedAspectRatio(W, H, FirstRelativeWeight, RelativeWeightOffset);
			float const RelativeWeight = ParentNode.Children[MidIdx]->Weight / AccumWeight;

			if (NormalizedAspectRatio(W, H, FirstRelativeWeight,
									  RelativeWeightOffset + RelativeWeight) > AspectRatio)
				break;

			MidIdx++;
			RelativeWeightOffset += RelativeWeight;
		}

		SliceAndDice(ParentNode, FirstIdx, MidIdx,
					 FBox2D(FVector2D(X, Y), FVector2D(X + W * RelativeWeightOffset, Y + H)));

		FVector2D const NextCornerMin(X + W * RelativeWeightOffset, Y);

		return Squarified(
			ParentNode, MidIdx + 1, LastIdx,
			FBox2D(NextCornerMin, NextCornerMin + FVector2D(W * (1 - RelativeWeightOffset), H)));
	}
}

void FIGVTreemapLayout::Squarified(FIGVTreemapNode& ParentNode, float Nesting)
{
	if (Nesting < 0)
	{
		Nesting = 0;
	}
	else if (Nesting > .4999)
	{
		Nesting = .4999;
	}

	ParentNode.SortChildrenBySize();
	FVector2D const BoundsSize = ParentNode.Rect.GetSize();
	Squarified(ParentNode, 0, ParentNode.Children.Num() - 1,
			   ParentNode.Rect.ExpandBy(FMath::Min(BoundsSize.X, BoundsSize.Y) * -Nesting * 0.5));
}

void FIGVTreemapLayout::Squarified(FIGVTreemapNode& ParentNode)
{
	ParentNode.SortChildrenBySize();
	Squarified(ParentNode, 0, ParentNode.Children.Num() - 1, ParentNode.Rect);
}

double FIGVTreemapLayout::AccumulateWeight(FIGVTreemapNode& ParentNode, int32 const FirstIdx,
										   int32 const LastIdx)
{
	check(LastIdx < ParentNode.Children.Num());

	double Result = 0;
	for (int32 Idx = FirstIdx; Idx <= LastIdx; Idx++)
	{
		Result += ParentNode.Children[Idx]->Weight;
	}
	return Result;
}

float FIGVTreemapLayout::NormalizedAspectRatio(float const Big, float const Small,
											   double const RelativeWeight,
											   double const WeightOffset)
{
	float const Ratio = (Big / (Small * RelativeWeight)) * WeightOffset * WeightOffset;
	return Ratio < 1.f ? 1.f / Ratio : Ratio;
}
