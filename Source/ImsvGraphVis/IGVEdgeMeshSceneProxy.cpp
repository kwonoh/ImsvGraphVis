// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "IGVEdgeMeshSceneProxy.h"

#include "IGVEdge.h"
#include "IGVEdgeMeshComponent.h"
#include "IGVGraphActor.h"
#include "IGVLog.h"

FIGVEdgeMeshVertexBuffer::FIGVEdgeMeshVertexBuffer(int32 const InNumElements)
	: NumElements(InNumElements)
{
}

void FIGVEdgeMeshVertexBuffer::InitRHI()
{
	checkSlow(NumElements > 0);
	FRHIResourceCreateInfo CreateInfo;
	VertexBufferRHI =
		RHICreateVertexBuffer(NumElements * sizeof(FDynamicMeshVertex),
							  BUF_UnorderedAccess | BUF_ByteAddressBuffer, CreateInfo);
}

void FIGVEdgeMeshVertexFactory::Init(FVertexBuffer* VertexBuffer)
{
	if (IsInRenderingThread())
	{
		Init_RenderThread(VertexBuffer);
	}
	else
	{
		FIGVEdgeMeshVertexFactory* VertexFactory = this;
		ENQUEUE_RENDER_COMMAND(InitMeshVertexFactory)(
					[VertexBuffer, VertexFactory](FRHICommandListImmediate& RHICmdList)
		{
			(void)RHICmdList;
			VertexFactory->Init_RenderThread(VertexBuffer);
		});
	}
}

void FIGVEdgeMeshVertexFactory::Init_RenderThread(const FVertexBuffer* VertexBuffer)
{
	FDataType NewData;
	NewData.PositionComponent =
		STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Position, VET_Float3);
	NewData.ColorComponent =
		STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Color, VET_Color);
	NewData.TextureCoordinates.Emplace(
		FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FDynamicMeshVertex, TextureCoordinate),
							   sizeof(FDynamicMeshVertex), VET_Float2));
	NewData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(
		VertexBuffer, FDynamicMeshVertex, TangentX, VET_PackedNormal);
	NewData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(
		VertexBuffer, FDynamicMeshVertex, TangentZ, VET_PackedNormal);
	SetData(NewData);
}

FIGVEdgeMeshSceneProxy::~FIGVEdgeMeshSceneProxy()
{
	VertexBuffer.ReleaseResource();
	IndexBuffer.ReleaseResource();
	VertexFactory.ReleaseResource();
	ReleaseBuffers();

	bIsComputeShaderUnloading = true;
}

FIGVEdgeMeshSceneProxy::FIGVEdgeMeshSceneProxy(UIGVEdgeMeshComponent* const Component)
	: FPrimitiveSceneProxy(Component),
	  IGVEdgeMeshComponent(Component),
	  GraphActor(Component->GraphActor),
	  RenderGroup(Component->RenderGroup),

	  SplineControlPointData(Component->SplineControlPointData),
	  SplineSegmentData(Component->SplineSegmentData),
	  SplineData(Component->SplineData),

	  InSplineControlPointBuffer(nullptr),
	  InSplineSegmentBuffer(nullptr),
	  InSplineBuffer(nullptr),

	  InSplineControlPointBufferSRV(nullptr),
	  InSplineSegmentBufferSRV(nullptr),
	  InSplineBufferSRV(nullptr),
	  OutMeshVertexBufferUAV(nullptr),

	  bIsComputeShaderExecuting(false),
	  bIsComputeShaderUnloading(false),

	  VertexBuffer(Component->NumMeshVertices),
	  IndexBuffer(Component->MeshIndices),
	  VertexFactory(ERHIFeatureLevel::SM5),

	  Material(Component->GetMaterial(0)),
	  MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel())),
	  WireframeMaterial(GEngine->WireframeMaterial
							? GEngine->WireframeMaterial->GetRenderProxy()
							: nullptr,
						FLinearColor::White)
{
	VertexFactory.Init(&VertexBuffer);
	BeginInitResource(&VertexBuffer);
	BeginInitResource(&IndexBuffer);
	BeginInitResource(&VertexFactory);

	if (Material == nullptr)
	{
		IGV_LOG(Warning, TEXT("Unable to find a material from UIGVEdgeMeshComponent"));
		Material = UMaterial::GetDefaultMaterial(MD_Surface);
	}

	if (SplineControlPointData.Num() > 0)
	{
		ComputeMesh();
	}
}

void FIGVEdgeMeshSceneProxy::OnTransformChanged()
{
	FBoxSphereBounds PreSkinnedLocalBounds;
	GetPreSkinnedLocalBounds(PreSkinnedLocalBounds);
	PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(
		GetLocalToWorld(), GetBounds(), GetLocalBounds(), PreSkinnedLocalBounds, ReceivesDecals(), DrawsVelocity(), LpvBiasMultiplier);
}

bool FIGVEdgeMeshSceneProxy::CanBeOccluded() const
{
	return !MaterialRelevance.bDisableDepthTest;
}

uint32 FIGVEdgeMeshSceneProxy::GetAllocatedSize() const
{
	return FPrimitiveSceneProxy::GetAllocatedSize();
}

uint32 FIGVEdgeMeshSceneProxy::GetMemoryFootprint() const
{
	return sizeof(*this) + GetAllocatedSize();
}

void FIGVEdgeMeshSceneProxy::SendRenderDynamicData()
{
	SplineControlPointData = IGVEdgeMeshComponent->SplineControlPointData;
	SplineSegmentData = IGVEdgeMeshComponent->SplineSegmentData;
	SplineData = IGVEdgeMeshComponent->SplineData;

	// TODO: is this required code?
	IndexBuffer.Indices = IGVEdgeMeshComponent->MeshIndices;

	FIGVEdgeMeshSceneProxy* Self = this;
	ENQUEUE_RENDER_COMMAND(InitMeshVertexFactory)(
				[Self](FRHICommandListImmediate& RHICmdList)
	{
		(void)RHICmdList;
		Self->SendRenderDynamicData_RenderThread();
	});

	ComputeMesh();
}

void FIGVEdgeMeshSceneProxy::SendRenderDynamicData_RenderThread()
{
}

void FIGVEdgeMeshSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views,
													const FSceneViewFamily& ViewFamily,
													uint32 VisibilityMap,
													class FMeshElementCollector& Collector) const
{
	bool const bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (VisibilityMap & (1 << ViewIndex))
		{
			FSceneView const* const View = Views[ViewIndex];

			if (VertexBuffer.NumElements > 0)
			{
				if (RenderGroup == EIGVEdgeRenderGroup::Default)
				{
					for (FIGVEdge& Edge : GraphActor->Edges)
					{
						if (Edge.RenderGroup == RenderGroup)
						{
							FMeshBatch& Mesh = Collector.AllocateMesh();
							SetMesh(Mesh, bWireframe);
							SetMeshBatchElement(Mesh.Elements[0], &Edge.MeshData);
							Collector.AddMesh(ViewIndex, Mesh);
						}
					}
				}
				else
				{
					FMeshBatch& Mesh = Collector.AllocateMesh();
					if (SetMeshBatchElements(Mesh, bWireframe))
					{
						Collector.AddMesh(ViewIndex, Mesh);
					}
				}
			}
		}
	}
}

void FIGVEdgeMeshSceneProxy::DrawStaticElements(FStaticPrimitiveDrawInterface* PDI)
{
	if (VertexBuffer.NumElements > 0)
	{
		FMeshBatch Mesh;
		if (SetMeshBatchElements(Mesh, false) > 0)
		{
			PDI->DrawMesh(Mesh, FLT_MAX);
		}
	}
}

FPrimitiveViewRelevance FIGVEdgeMeshSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = VertexBuffer.NumElements > 0 && IsShown(View);
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();
	Result.bShadowRelevance = IsShadowCast(View);
	Result.bDynamicRelevance = RenderGroup == EIGVEdgeRenderGroup::Default;
	// Result.bDynamicRelevance = View->Family->EngineShowFlags.Wireframe || IsSelected();
	Result.bStaticRelevance = !Result.bDynamicRelevance;
	MaterialRelevance.SetPrimitiveViewRelevance(Result);
	return Result;
}

void FIGVEdgeMeshSceneProxy::SetMesh(FMeshBatch& Mesh, bool const bWireframe) const
{
	Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
	Mesh.Type = PT_TriangleList;
	Mesh.DepthPriorityGroup = SDPG_World;
	Mesh.bCanApplyViewModeOverrides = false;
	Mesh.bWireframe = bWireframe;
	Mesh.VertexFactory = &VertexFactory;
	Mesh.MaterialRenderProxy =
		!bWireframe ? Material->GetRenderProxy() : &WireframeMaterial;
}

int32 FIGVEdgeMeshSceneProxy::SetMeshBatchElements(FMeshBatch& Mesh, bool const bWireframe) const
{
	SetMesh(Mesh, bWireframe);

	int32 NumBatchElement = 0;

	if (RenderGroup == EIGVEdgeRenderGroup::Default)
	{
		FMeshBatchElement* BatchElement = nullptr;

		for (FIGVEdge& Edge : GraphActor->Edges)
		{
			// if (Edge.bHidden) continue;
			if (Edge.RenderGroup == RenderGroup)
			{
				FIGVEdgeMeshData& MeshData = Edge.MeshData;

				if (BatchElement == nullptr)
				{
					BatchElement = (NumBatchElement > 0) ? (new (Mesh.Elements) FMeshBatchElement)
														 : &Mesh.Elements[0];
					SetMeshBatchElement(*BatchElement, &MeshData);
					NumBatchElement++;
				}
				else
				{
					BatchElement->NumPrimitives += MeshData.IndexBufferSize[RenderGroup] / 3;
					BatchElement->MaxVertexIndex += MeshData.IndexBufferSize[RenderGroup];
				}
			}
			else
			{
				BatchElement = nullptr;
			}
		}
	}
	else
	{
		FMeshBatchElement& BatchElement = Mesh.Elements[0];
		BatchElement.IndexBuffer = &IndexBuffer;
		BatchElement.PrimitiveUniformBuffer = PrimitiveUniformBuffer;
		BatchElement.FirstIndex = 0;
		BatchElement.NumPrimitives = IndexBuffer.NumElements / 3;
		BatchElement.MinVertexIndex = 0;
		BatchElement.MaxVertexIndex = VertexBuffer.NumElements - 1;
		NumBatchElement++;
	}

	return NumBatchElement;
}

void FIGVEdgeMeshSceneProxy::SetMeshBatchElement(FMeshBatchElement& BatchElement,
												 FIGVEdgeMeshData* const MeshData) const
{
	BatchElement.IndexBuffer = &IndexBuffer;
	BatchElement.PrimitiveUniformBuffer = PrimitiveUniformBuffer;

	BatchElement.FirstIndex = MeshData->IndexBufferOffset[RenderGroup];
	BatchElement.NumPrimitives = MeshData->IndexBufferSize[RenderGroup] / 3;
	BatchElement.MinVertexIndex = MeshData->VertexBufferOffset[RenderGroup];
	BatchElement.MaxVertexIndex =
		MeshData->VertexBufferOffset[RenderGroup] + MeshData->VertexBufferSize[RenderGroup] - 1;
}

SIZE_T FIGVEdgeMeshSceneProxy::GetTypeHash() const {
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}

void FIGVEdgeMeshSceneProxy::ReleaseBuffers()
{
	if (InSplineControlPointBuffer != nullptr)
	{
		InSplineControlPointBuffer.SafeRelease();
	}

	if (InSplineSegmentBuffer != nullptr)
	{
		InSplineSegmentBuffer.SafeRelease();
	}

	if (InSplineBuffer != nullptr)
	{
		InSplineBuffer.SafeRelease();
	}

	if (InSplineControlPointBufferSRV != nullptr)
	{
		InSplineControlPointBufferSRV.SafeRelease();
	}

	if (InSplineSegmentBufferSRV != nullptr)
	{
		InSplineSegmentBufferSRV.SafeRelease();
	}

	if (InSplineBufferSRV != nullptr)
	{
		InSplineBufferSRV.SafeRelease();
	}

	if (OutMeshVertexBufferUAV != nullptr)
	{
		OutMeshVertexBufferUAV.SafeRelease();
	}
}
void FIGVEdgeMeshSceneProxy::CreateBuffers()
{
	uint32 const SplineControlPointBufferByteSize =
		sizeof(FIGVEdgeSplineControlPointData) * SplineControlPointData.Num();
	uint32 const SplineSegmentBufferByteSize =
		sizeof(FIGVEdgeSplineSegmentData) * SplineSegmentData.Num();
	uint32 const SplineBufferByteSize = sizeof(FIGVEdgeSplineData) * SplineData.Num();

	FRHIResourceCreateInfo CreateInfo;

	if (!(InSplineControlPointBuffer != nullptr))
	{
		InSplineControlPointBuffer = RHICreateStructuredBuffer(
			sizeof(FIGVEdgeSplineControlPointData), SplineControlPointBufferByteSize,
			BUF_ShaderResource, CreateInfo);
		InSplineControlPointBufferSRV = RHICreateShaderResourceView(InSplineControlPointBuffer);
	}

	if (!(InSplineSegmentBuffer != nullptr))
	{
		InSplineSegmentBuffer =
			RHICreateStructuredBuffer(sizeof(FIGVEdgeSplineSegmentData),
									  SplineSegmentBufferByteSize, BUF_ShaderResource, CreateInfo);
		InSplineSegmentBufferSRV = RHICreateShaderResourceView(InSplineSegmentBuffer);
	}

	if (!(InSplineBuffer != nullptr))
	{
		InSplineBuffer = RHICreateStructuredBuffer(sizeof(FIGVEdgeSplineData), SplineBufferByteSize,
												   BUF_ShaderResource, CreateInfo);
		InSplineBufferSRV = RHICreateShaderResourceView(InSplineBuffer);
	}

	if (!(OutMeshVertexBufferUAV != nullptr))
	{
		OutMeshVertexBufferUAV =
			RHICreateUnorderedAccessView(VertexBuffer.VertexBufferRHI, PF_R32_UINT);
	}
}

void FIGVEdgeMeshSceneProxy::ComputeMesh()
{
	if (bIsComputeShaderUnloading || bIsComputeShaderExecuting)
	{
		return;
	}

	bIsComputeShaderExecuting = true;

	SplineComputeShaderUniformParameters.WorldSize = GraphActor->GetSphereRadius();
	SplineComputeShaderUniformParameters.Width = GraphActor->EdgeWidth * 0.5;
	SplineComputeShaderUniformParameters.NumSides = GraphActor->EdgeNumSides;

	FIGVEdgeMeshSceneProxy* Self = this;
	ENQUEUE_RENDER_COMMAND(FSendIGVEdgeMeshDynamicData)(
				[Self](FRHICommandListImmediate& RHICmdList)
	{
		(void)RHICmdList;
		Self->ComputeMesh_RenderThread();
	});
}

void FIGVEdgeMeshSceneProxy::ComputeMesh_RenderThread()
{
	check(IsInRenderingThread());

	CreateBuffers();

	uint32 const SplineControlPointBufferByteSize =
		sizeof(FIGVEdgeSplineControlPointData) * SplineControlPointData.Num();
	uint32 const SplineSegmentBufferByteSize =
		sizeof(FIGVEdgeSplineSegmentData) * SplineSegmentData.Num();
	uint32 const SplineBufferByteSize = sizeof(FIGVEdgeSplineData) * SplineData.Num();

	// Spline Control Points
	void* SplineControlPointBuffer = RHILockStructuredBuffer(
		InSplineControlPointBuffer, 0, SplineControlPointBufferByteSize, RLM_WriteOnly);
	FMemory::Memcpy(SplineControlPointBuffer, SplineControlPointData.GetData(),
					SplineControlPointBufferByteSize);
	RHIUnlockStructuredBuffer(InSplineControlPointBuffer);

	// Spline Segments
	void* SplineSegmentBuffer = RHILockStructuredBuffer(InSplineSegmentBuffer, 0,
														SplineSegmentBufferByteSize, RLM_WriteOnly);
	FMemory::Memcpy(SplineSegmentBuffer, SplineSegmentData.GetData(), SplineSegmentBufferByteSize);
	RHIUnlockStructuredBuffer(InSplineSegmentBuffer);

	// Spline
	void* SplineBuffer =
		RHILockStructuredBuffer(InSplineBuffer, 0, SplineBufferByteSize, RLM_WriteOnly);
	FMemory::Memcpy(SplineBuffer, SplineData.GetData(), SplineBufferByteSize);
	RHIUnlockStructuredBuffer(InSplineBuffer);

	// Compute Shader
	if (bIsComputeShaderUnloading)
	{
		ReleaseBuffers();
		return;
	}

	FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();

	// TODO: Fix occasional crash
	TShaderMapRef<FSplineComputeShader_Sphere> ComputeShader(
		GetGlobalShaderMap(GraphActor->GetWorld()->Scene->GetFeatureLevel()));
	RHICmdList.SetComputeShader(ComputeShader->GetComputeShader());
	ComputeShader->SetBuffers(RHICmdList, InSplineControlPointBufferSRV, InSplineSegmentBufferSRV,
							  InSplineBufferSRV, OutMeshVertexBufferUAV);
	ComputeShader->SetUniformBuffers(RHICmdList, SplineComputeShaderUniformParameters);
	DispatchComputeShader(RHICmdList, *ComputeShader, SplineSegmentData.Num(), 1, 1);
	ComputeShader->UnbindBuffers(RHICmdList);

	bIsComputeShaderExecuting = false;
}
