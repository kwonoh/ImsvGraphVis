// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "IGVCluster.h"

#include "IGVGraphActor.h"
#include "IGVLog.h"
#include "IGVNodeActor.h"

FIGVCluster::FIGVCluster(AIGVGraphActor* const InGraphActor)
	: GraphActor(InGraphActor),
	  Idx(-1),
	  NodeIdx(-1),
	  ParentIdx(-1),
	  Height(-1),
	  Pos2D(FVector2D::ZeroVector),
	  Pos3D(FVector::ZeroVector),
	  Parent(nullptr),
	  Children(),
	  Node(nullptr),
	  NumDescendantNodes(0)
{
}

FString FIGVCluster::ToString() const
{
	return FString::Printf(
		TEXT("Idx=%d NodeIdx=%d ParentIdx=%d Height=%d NumChildren=%d Pos2D=(%s) Pos3D=(%s)"), Idx,
		NodeIdx, ParentIdx, Height, Children.Num(), *Pos2D.ToString(), *Pos3D.ToString());
}

void FIGVCluster::SetNumDescendantNodes()
{
	ForEachDescendantFirst([](FIGVCluster& Cluster) {
		if (Cluster.IsLeaf())
		{
			Cluster.NumDescendantNodes = 1;
		}
		else
		{
			for (FIGVCluster* const Child : Cluster.Children)
			{
				Cluster.NumDescendantNodes += Child->NumDescendantNodes;
			}
		}
	});
}

bool FIGVCluster::IsRoot() const
{
	return ParentIdx == -1;
}

bool FIGVCluster::IsLeaf() const
{
	return NodeIdx != -1;
}

void FIGVCluster::SetPos2D(FVector2D const& P)
{
	Pos2D = P;
	if (Node)
	{
		Node->Pos2D = P;
	}
}

void FIGVCluster::SetPosNonLeaf()
{
	ForEachDescendantFirst([](FIGVCluster& Cluster) {
		if (Cluster.IsLeaf())
		{
			Cluster.Pos3D = Cluster.Node->Pos3D;
		}
		else
		{
			Cluster.Pos3D = FVector::ZeroVector;
			for (FIGVCluster* const Child : Cluster.Children)
			{
				Cluster.Pos3D += Child->Pos3D * Child->NumDescendantNodes;
			}
			Cluster.Pos3D.Normalize();
		}
	});
}

float FIGVCluster::DefaultLevel() const
{
	return (1 + GraphActor->ClusterLevelOffset) +
		   GraphActor->ClusterLevelScale * FMath::Pow(Height - 1, GraphActor->ClusterLevelExponent);
}
