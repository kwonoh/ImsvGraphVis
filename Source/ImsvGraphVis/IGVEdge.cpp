// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "IGVEdge.h"

#include "IGVCluster.h"
#include "IGVGraphActor.h"
#include "IGVLog.h"
#include "IGVNodeActor.h"

FIGVEdge::FIGVEdge(AIGVGraphActor* const InGraphActor)
	: GraphActor(InGraphActor),
	  SourceIdx(-1),
	  TargetIdx(-1),
	  SourceNode(nullptr),
	  TargetNode(nullptr),
	  LowestCommonAncestor(nullptr),
	  Clusters(),
	  ClusterLevels(),
	  SplineControlPointData(),
	  MeshData(),
	  RenderGroup(EIGVEdgeRenderGroup::Default),
	  bUpdateMeshRequired(false)
{
}

FString FIGVEdge::ToString() const
{
	return FString::Printf(TEXT("SourceIdx=%d TargetIdx=%d"), SourceIdx, TargetIdx);
}

void FIGVEdge::SetupClusters()
{
	TArray<int32> const& SourceAncIdxs = SourceNode->AncIdxs;
	TArray<int32> const& TargetAncIdxs = TargetNode->AncIdxs;

	// The number of ancestors depends on the clustering algorithm. We used python-louvain for this
	// implementation. See /Preprocess directory for more details.
	check(SourceAncIdxs.Num() == TargetAncIdxs.Num());

	// Find the lowest common ancestor in the clustering hierarchy
	int32 LCAIdx = GraphActor->RootCluster->Idx;
	int32 LCAIdxInAncs = 0;
	for (int32 Idx = 0, Num = SourceAncIdxs.Num(); Idx < Num; Idx++)
	{
		int32 const SourceAncestorIdx = SourceAncIdxs[Idx];
		int32 const TargetAncestorIdx = TargetAncIdxs[Idx];
		if (SourceAncestorIdx == TargetAncestorIdx)
		{
			LCAIdx = SourceAncestorIdx;
			break;
		}
		LCAIdxInAncs++;
	}
	LowestCommonAncestor = &GraphActor->Clusters[LCAIdx];
	check(LowestCommonAncestor->Idx == LCAIdx);

	// Get the path in the clustering hierarchy
	for (int32 Idx = 0; Idx < LCAIdxInAncs; Idx++)
	{
		Clusters.Add(&GraphActor->Clusters[SourceAncIdxs[Idx]]);
	}
	LowestCommonAncestorIdxInClusters = Clusters.Num();
	Clusters.Add(LowestCommonAncestor);
	for (int32 Idx = LCAIdxInAncs - 1; Idx >= 0; Idx--)
	{
		Clusters.Add(&GraphActor->Clusters[TargetAncIdxs[Idx]]);
	}

	UpdateDefaultClusterLevels();
	ClusterLevels = ClusterLevelsBeforeTransition = ClusterLevelsAfterTransition =
		ClusterLevelsDefault;

	// Debug
	TArray<FString> ClusterStrs;
	for (FIGVCluster* const Cluster : Clusters)
	{
		ClusterStrs.Add(FString::FromInt(Cluster->Idx));
	}

	IGV_LOG(Log, TEXT("LowestCommonAncestor.Idx=%d Clusters=[%s] Source=%s Target=%s"), LCAIdx,
			*FString::Join(ClusterStrs, TEXT(" ")), *SourceNode->ToString(),
			*TargetNode->ToString());
}

void FIGVEdge::UpdateDefaultClusterLevels()
{
	ClusterLevelsDefault.Reset();
	for (FIGVCluster* const Cluster : Clusters)
	{
		ClusterLevelsDefault.Emplace(Cluster->DefaultLevel());
	}
}

void FIGVEdge::UpdateSplineControlPoints()
{
	UpdateSplineControlPointsImpl(ClusterLevels, SourceNode->LevelScale, TargetNode->LevelScale);
}

void FIGVEdge::UpdateDefaultSplineControlPoints()
{
	UpdateDefaultClusterLevels();
	UpdateSplineControlPointsImpl(ClusterLevelsDefault, GraphActor->DefaultLevelScale,
								  GraphActor->DefaultLevelScale);
}

void FIGVEdge::UpdateSplineControlPointsImpl(TArray<float> const& InClusterLevels,
											 float const SourceLevelScale,
											 float const TargetLevelScale)
{
	SplineControlPointData.Reset();

	SplineControlPointData.Emplace(
		FIGVEdgeSplineControlPointData{SourceNode->Pos3D, SourceLevelScale, 0.0});

	for (int32 Idx = 0, NumPath = Clusters.Num(); Idx < NumPath; Idx++)
	{
		FIGVCluster* const Cluster = Clusters[Idx];

		if (!Cluster->IsRoot())
		{
			float const Alpha = float(Idx + 1) / float(NumPath + 1);
			SplineControlPointData.Emplace(
				FIGVEdgeSplineControlPointData{Cluster->Pos3D, InClusterLevels[Idx], Alpha});
		}
	}

	SplineControlPointData.Emplace(
		FIGVEdgeSplineControlPointData{TargetNode->Pos3D, TargetLevelScale, 1.0});
}

float FIGVEdge::BundlingStrength() const
{
	// TODO: individual BundlingStrength
	return GraphActor->EdgeBundlingStrength;
}

bool FIGVEdge::HasHighlightedNode() const
{
	return (SourceNode->bIsHighlighted || TargetNode->bIsHighlighted);
}

bool FIGVEdge::HasNeighborHighlightedNode() const
{
	return (SourceNode->HasHighlightedNeighbor() || TargetNode->HasHighlightedNeighbor());
}

bool FIGVEdge::HasBothHighlightedNodes() const
{
	return (SourceNode->bIsHighlighted && TargetNode->bIsHighlighted);
}

bool FIGVEdge::IsDefaultRenderGroup() const
{
	return !(bInTransition || HasHighlightedNode() || HasNeighborHighlightedNode());
}

bool FIGVEdge::IsHighlightedRenderGroup() const
{
	return HasBothHighlightedNodes() || (HasHighlightedNode() && HasNeighborHighlightedNode());
}

bool FIGVEdge::IsRemainedRenderGroup() const
{
	return !(IsDefaultRenderGroup() || IsHighlightedRenderGroup());
}

void FIGVEdge::UpdateRenderGroup()
{
	if (IsHighlightedRenderGroup())
	{
		RenderGroup = EIGVEdgeRenderGroup::Highlighted;
	}
	else if (IsDefaultRenderGroup())
	{
		RenderGroup = EIGVEdgeRenderGroup::Default;
	}
	else
	{
		RenderGroup = EIGVEdgeRenderGroup::Remained;
	}
}

void FIGVEdge::BeginTransition()
{
	ClusterLevelsBeforeTransition = ClusterLevels;

	float const SouceLevelScaleAfterTransition = SourceNode->LevelScaleAfterTransition;
	float const TargetLevelScaleAfterTransition = TargetNode->LevelScaleAfterTransition;

	ClusterLevelsAfterTransition.Reset();

	if (HasHighlightedNode() || HasNeighborHighlightedNode())
	{
		float const HighlightedLevelScaleOfLowestCommonAncestor =
			HasBothHighlightedNodes()
				? GraphActor->HighlightedLevelScale
				: GraphActor->DefaultLevelScale + GraphActor->ClusterLevelOffset;

		for (int32 Idx = 0; Idx < LowestCommonAncestorIdxInClusters; Idx++)
		{
			ClusterLevelsAfterTransition.Emplace(FMath::Lerp(
				SouceLevelScaleAfterTransition, HighlightedLevelScaleOfLowestCommonAncestor,
				float(Idx + 1) / float(LowestCommonAncestorIdxInClusters + 1)));
		}

		ClusterLevelsAfterTransition.Emplace(HighlightedLevelScaleOfLowestCommonAncestor);

		for (int32 Idx = LowestCommonAncestorIdxInClusters + 1, Num = ClusterLevels.Num();
			 Idx < Num; Idx++)
		{
			ClusterLevelsAfterTransition.Emplace(FMath::Lerp(
				HighlightedLevelScaleOfLowestCommonAncestor, TargetLevelScaleAfterTransition,
				float(Idx - LowestCommonAncestorIdxInClusters) /
					float(Num - LowestCommonAncestorIdxInClusters)));
		}
	}
	else
	{
		for (FIGVCluster* const Cluster : Clusters)
		{
			ClusterLevelsAfterTransition.Emplace(Cluster->DefaultLevel());
		}
	}
}

void FIGVEdge::OnHighlightTransitionTimelineUpdate(ETimelineDirection::Type const Direction,
												   float const Alpha)
{
	bInTransition = true;
	bUpdateMeshRequired = true;

	for (int32 Idx = 0, Num = ClusterLevels.Num(); Idx < Num; Idx++)
	{
		ClusterLevels[Idx] = FMath::Lerp(ClusterLevelsBeforeTransition[Idx],
										 ClusterLevelsAfterTransition[Idx], Alpha);
	}
}

void FIGVEdge::OnHighlightTransitionTimelineFinished(ETimelineDirection::Type const Direction)
{
	bInTransition = false;
}
