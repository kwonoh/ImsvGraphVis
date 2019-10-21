// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "IGVEdgeMeshComponent.h"

#include "KWColorSpace.h"
#include "KWTask.h"

#include "IGVEdge.h"
#include "IGVEdgeMeshSceneProxy.h"
#include "IGVGraphActor.h"
#include "IGVLog.h"
#include "IGVNodeActor.h"

UMaterialInterface* GetEdgeMaterial()
{
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(
		TEXT("/Game/Materials/M_Edge.M_Edge"));
	return MaterialAsset.Succeeded() ? MaterialAsset.Object->GetMaterial() : nullptr;
}

UIGVEdgeMeshComponent::UIGVEdgeMeshComponent()
	: GraphActor(nullptr),
	  RenderGroup(EIGVEdgeRenderGroup::Default),
	  SplineControlPointData(),
	  SplineSegmentData(),
	  SplineData(),
	  NumMeshIndices(0),
	  NumMeshVertices(0),
	  MeshIndices(),
	  MaterialInstance(nullptr)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	GetEdgeMaterial();
}

void UIGVEdgeMeshComponent::Init(AIGVGraphActor* const InGraphActor,
								 EIGVEdgeRenderGroup::Type const InRenderGroup)
{
	GraphActor = InGraphActor;
	RenderGroup = InRenderGroup;

	MaterialInstance = UMaterialInstanceDynamic::Create(GetEdgeMaterial(), this);
	if (MaterialInstance != nullptr) SetMaterial(0, MaterialInstance);
}

void UIGVEdgeMeshComponent::Setup()
{
	SplineControlPointData.Empty();
	SplineSegmentData.Empty();
	SplineData.Empty();
	MeshIndices.Empty();

	if (RenderGroup == EIGVEdgeRenderGroup::Highlighted)
	{
		SetHalo(true);
	}

	Update();
}

int32 GetVertexIdx(int32 const NumSides, int32 const AlongIdx, int32 const AroundIdx)
{
	return (AlongIdx * NumSides) + (AroundIdx % NumSides);
}

void UIGVEdgeMeshComponent::Update()
{
	SplineControlPointData.Reset();
	SplineSegmentData.Reset();
	SplineData.Reset();
	MeshIndices.Reset();

	NumMeshVertices = 0;
	NumMeshIndices = 0;

	uint32 const NumSides = GraphActor->EdgeNumSides;
	uint32 const NumSegmentSamples = GraphActor->EdgeSplineResolution;

	for (FIGVEdge& Edge : GraphActor->Edges)
	{
		if (!(RenderGroup == EIGVEdgeRenderGroup::Default || Edge.RenderGroup == RenderGroup))
			continue;

		uint32 const BeginControlPointIdx = SplineControlPointData.Num();
		uint32 const EdgeMeshVertexBufferOffset = NumMeshVertices;
		uint32 const EdgeMeshIndexBufferOffset = NumMeshIndices;
		uint32 const SplineIdx = SplineData.Num();

		uint32 const NumSplineControlPoints = Edge.SplineControlPointData.Num();
		SplineControlPointData.AddUninitialized(NumSplineControlPoints + 4);

		// NumSplineControlPoints + 3 - 2. Degree - First and Last Control Point
		uint32 const NumSplineSegments = NumSplineControlPoints + 1;

		for (uint32 SegmentIdx = 0; SegmentIdx < NumSplineSegments; SegmentIdx++)
		{
			uint32 const SegmentMeshVertexBufferOffset = NumMeshVertices;
			uint32 const SegmentMeshIndexBufferOffset = NumMeshIndices;

			SplineSegmentData.Emplace(
				FIGVEdgeSplineSegmentData{SplineIdx,						  //
										  BeginControlPointIdx + SegmentIdx,  //
										  NumSegmentSamples,				  //
										  SegmentMeshVertexBufferOffset,	  //
										  SegmentMeshIndexBufferOffset});

			uint32 const NumSegmentMeshIndices =
				(NumSegmentSamples - 1) * NumSides * 6;  // Two Triangles
			MeshIndices.AddUninitialized(NumSegmentMeshIndices);

			NumMeshVertices += NumSides * NumSegmentSamples;
			NumMeshIndices += NumSegmentMeshIndices;  // Two Triangles
		}

		AIGVNodeActor const& SourceNode = *Edge.SourceNode;
		AIGVNodeActor const& TargetNode = *Edge.TargetNode;
		SplineData.Emplace(
			FIGVEdgeSplineData{SourceNode.Pos3D,									 //
							   TargetNode.Pos3D,									 //
							   UKWColorSpace::RGBtoHCL(SourceNode.Color),			 //
							   UKWColorSpace::RGBtoHCL(TargetNode.Color),			 //
							   Edge.BundlingStrength(),								 //
							   BeginControlPointIdx,								 //
							   SplineControlPointData.Num() - BeginControlPointIdx,  //
							   EdgeMeshVertexBufferOffset});

		FIGVEdgeMeshData& MeshData = Edge.MeshData;
		MeshData.VertexBufferOffset[RenderGroup] = EdgeMeshVertexBufferOffset;
		MeshData.VertexBufferSize[RenderGroup] = NumMeshVertices - EdgeMeshVertexBufferOffset;
		MeshData.IndexBufferOffset[RenderGroup] = EdgeMeshIndexBufferOffset;
		MeshData.IndexBufferSize[RenderGroup] = NumMeshIndices - EdgeMeshIndexBufferOffset;
	}

	FGraphEventArray Tasks;
	uint32 SplineIdx = 0;
	for (FIGVEdge& Edge : GraphActor->Edges)
	{
		if (!(RenderGroup == EIGVEdgeRenderGroup::Default || Edge.RenderGroup == RenderGroup))
			continue;

		FIGVEdgeSplineData& Spline = SplineData[SplineIdx];
		SplineIdx++;

		Tasks.Add(FKWTask<>::ConstructAndDispatchWhenReady([&] {
			auto const& ControlPoints = Edge.SplineControlPointData;

			uint32 const I = Spline.BeginControlPointIdx;
			uint32 const J = I + Spline.NumControlPoints - 1;

			SplineControlPointData[I] = SplineControlPointData[I + 1] = ControlPoints[0];
			FMemory::Memcpy(&SplineControlPointData[I + 2], ControlPoints.GetData(),
							sizeof(FIGVEdgeSplineControlPointData) * ControlPoints.Num());
			SplineControlPointData[J - 1] = SplineControlPointData[J] = ControlPoints.Last();
		}));
	}

	for (FIGVEdgeSplineSegmentData& Segment : SplineSegmentData)
	{
		Tasks.Add(FKWTask<>::ConstructAndDispatchWhenReady([&] {
			uint32 const SegmentMeshVertexBufferOffset = Segment.MeshVertexBufferOffset;
			uint32 const SegmentMeshIndexBufferOffset = Segment.MeshIndexBufferOffset;

			uint32 Idx = 0;
			for (uint32 SampleIdx = 0; SampleIdx < Segment.NumSamples - 1; SampleIdx++)
			{
				for (uint32 SideIdx = 0; SideIdx < NumSides; SideIdx++)
				{
					uint32 const TopLeft =
						SegmentMeshVertexBufferOffset + GetVertexIdx(NumSides, SampleIdx, SideIdx);
					uint32 const BottomLeft = SegmentMeshVertexBufferOffset +
											  GetVertexIdx(NumSides, SampleIdx, SideIdx + 1);
					uint32 const TopRight = SegmentMeshVertexBufferOffset +
											GetVertexIdx(NumSides, SampleIdx + 1, SideIdx);
					uint32 const BottomRight = SegmentMeshVertexBufferOffset +
											   GetVertexIdx(NumSides, SampleIdx + 1, SideIdx + 1);

					MeshIndices[SegmentMeshIndexBufferOffset + Idx] = TopLeft;
					MeshIndices[SegmentMeshIndexBufferOffset + Idx + 1] = BottomLeft;
					MeshIndices[SegmentMeshIndexBufferOffset + Idx + 2] = TopRight;

					MeshIndices[SegmentMeshIndexBufferOffset + Idx + 3] = TopRight;
					MeshIndices[SegmentMeshIndexBufferOffset + Idx + 4] = BottomLeft;
					MeshIndices[SegmentMeshIndexBufferOffset + Idx + 5] = BottomRight;

					Idx += 6;
				}
			}
		}));
	}
	FTaskGraphInterface::Get().WaitUntilTasksComplete(Tasks);

	check(NumMeshIndices == MeshIndices.Num());

	MarkRenderStateDirty();

	// IGV_LOG(Log, TEXT("UIGVEdgeMeshComponent::Update (%s) NumSplineSegmentData=%d"),
	// 		*UEnum::GetValueAsString(TEXT("ImsvGraphVis.EIGVEdgeRenderGroup"), RenderGroup),
	// 		SplineSegmentData.Num());
}

FIGVEdgeMeshSceneProxy* UIGVEdgeMeshComponent::GetSceneProxy() const
{
	return (FIGVEdgeMeshSceneProxy*)SceneProxy;
}

void UIGVEdgeMeshComponent::SendRenderDynamicData_Concurrent()
{
	if (SceneProxy)
	{
		GetSceneProxy()->SendRenderDynamicData();
	}
}

FBoxSphereBounds UIGVEdgeMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds NewBounds;
	NewBounds.Origin = FVector::ZeroVector;
	NewBounds.BoxExtent = FVector(HALF_WORLD_MAX, HALF_WORLD_MAX, HALF_WORLD_MAX);
	NewBounds.SphereRadius = FMath::Sqrt(3.0f * FMath::Square(HALF_WORLD_MAX));
	return NewBounds;
}

FPrimitiveSceneProxy* UIGVEdgeMeshComponent::CreateSceneProxy()
{
	return SplineControlPointData.Num() > 0 ? new FIGVEdgeMeshSceneProxy(this) : nullptr;
}

void UIGVEdgeMeshComponent::SetHalo(bool const bValue)
{
	SetRenderCustomDepth(bValue);
}
