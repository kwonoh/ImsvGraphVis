// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "KWMeshElement.h"
#include "SplineComputeShader.h"

#include "IGVEdgeMeshData.h"
#include "IGVEdgeSplineData.h"

class IMSVGRAPHVIS_API FIGVEdgeMeshVertexBuffer : public FVertexBuffer
{
public:
	int32 const NumElements;

	FIGVEdgeMeshVertexBuffer(int32 const InNumElements);
	virtual void InitRHI() override;
};

class IMSVGRAPHVIS_API FIGVEdgeMeshVertexFactory : public FLocalVertexFactory
{
public:
	void Init(FVertexBuffer* VertexBuffer);
	void Init_RenderThread(const FVertexBuffer* VertexBuffer);
};

class IMSVGRAPHVIS_API FIGVEdgeMeshSceneProxy : public FPrimitiveSceneProxy
{
public:
	class UIGVEdgeMeshComponent* IGVEdgeMeshComponent;
	class AIGVGraphActor* GraphActor;
	EIGVEdgeRenderGroup::Type const RenderGroup;

	TArray<FIGVEdgeSplineControlPointData> SplineControlPointData;
	TArray<FIGVEdgeSplineSegmentData> SplineSegmentData;
	TArray<FIGVEdgeSplineData> SplineData;

	FStructuredBufferRHIRef InSplineControlPointBuffer;
	FStructuredBufferRHIRef InSplineSegmentBuffer;
	FStructuredBufferRHIRef InSplineBuffer;

	FShaderResourceViewRHIRef InSplineControlPointBufferSRV;
	FShaderResourceViewRHIRef InSplineSegmentBufferSRV;
	FShaderResourceViewRHIRef InSplineBufferSRV;
	FUnorderedAccessViewRHIRef OutMeshVertexBufferUAV;

	FSplineComputeShaderUniformParameters SplineComputeShaderUniformParameters;

	bool bIsComputeShaderExecuting;
	bool bIsComputeShaderUnloading;

	FIGVEdgeMeshVertexBuffer VertexBuffer;
	FResourceArrayIndexBuffer IndexBuffer;
	FIGVEdgeMeshVertexFactory VertexFactory;

	UMaterialInterface* Material;
	FMaterialRelevance MaterialRelevance;
	FColoredMaterialRenderProxy WireframeMaterial;
	TUniformBufferRef<FPrimitiveUniformShaderParameters> PrimitiveUniformBuffer;

public:
	virtual ~FIGVEdgeMeshSceneProxy();

	FIGVEdgeMeshSceneProxy(class UIGVEdgeMeshComponent* const Component);

public:
	virtual void OnTransformChanged() override;
	virtual bool CanBeOccluded() const override;
	uint32 GetAllocatedSize() const;
	virtual uint32 GetMemoryFootprint() const override;

public:
	void SendRenderDynamicData();
	void SendRenderDynamicData_RenderThread();

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views,
										const FSceneViewFamily& ViewFamily, uint32 VisibilityMap,
										class FMeshElementCollector& Collector) const override;
	virtual void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI) override;
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;

	void SetMesh(FMeshBatch& Mesh, bool const bWireframe) const;
	int32 SetMeshBatchElements(FMeshBatch& Mesh, bool const bWireframe) const;
	void SetMeshBatchElement(FMeshBatchElement& BatchElement,
							 FIGVEdgeMeshData* const MeshData) const;

private:
	void ReleaseBuffers();
	void CreateBuffers();

	void ComputeMesh();
	void ComputeMesh_RenderThread();
};
