// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "Engine.h"

#include "DynamicMeshBuilder.h"
#include "RenderingThread.h"

typedef TArray<FDynamicMeshVertex, TAlignedHeapAllocator<VERTEXBUFFER_ALIGNMENT>> FMeshVertexArray;
typedef TResourceArray<FDynamicMeshVertex, VERTEXBUFFER_ALIGNMENT> FMeshVertexResourceArray;

typedef TArray<int32, TAlignedHeapAllocator<INDEXBUFFER_ALIGNMENT>> FMeshIndexArray;
typedef TResourceArray<int32, INDEXBUFFER_ALIGNMENT> FMeshIndexResourceArray;

class FSimpleVertexResourceArray : public FResourceArrayInterface
{
public:
	FSimpleVertexResourceArray(void* InData, uint32 InSize) : Data(InData), Size(InSize)
	{
	}

	virtual const void* GetResourceData() const override
	{
		return Data;
	}
	virtual uint32 GetResourceDataSize() const override
	{
		return Size;
	}
	virtual void Discard() override
	{
	}
	virtual bool IsStatic() const override
	{
		return false;
	}
	virtual bool GetAllowCPUAccess() const override
	{
		return false;
	}
	virtual void SetAllowCPUAccess(bool bInNeedsCPUAccess) override
	{
	}

private:
	void* Data;
	uint32 Size;
};

class FSimpleVertexBuffer : public FVertexBuffer
{
public:
	FMeshVertexArray Vertices;
	int32 const NumElements;
	int32 const BufferUsage;

	FSimpleVertexBuffer(FMeshVertexArray const& InVertices, int32 const InBufferUsage = BUF_Static)
		: Vertices(InVertices), NumElements(InVertices.Num()), BufferUsage(InBufferUsage)
	{
	}

	FSimpleVertexBuffer(int32 const InNumElements, FMeshVertexArray const& InVertices,
						int32 const InBufferUsage = BUF_Static)
		: Vertices(InVertices), NumElements(InNumElements), BufferUsage(InBufferUsage)
	{
	}

	virtual void InitRHI() override
	{
		checkSlow(NumElements > 0);
		FRHIResourceCreateInfo CreateInfo;
		VertexBufferRHI = RHICreateVertexBuffer(NumElements * sizeof(FDynamicMeshVertex),
												BufferUsage, CreateInfo);
		BufferData();
	}

	void BufferData()
	{
		if (Vertices.Num() > 0)
		{
			void* Buffer = RHILockVertexBuffer(
				VertexBufferRHI, 0, Vertices.Num() * sizeof(FDynamicMeshVertex), RLM_WriteOnly);
			FMemory::Memcpy(Buffer, Vertices.GetData(),
							Vertices.Num() * sizeof(FDynamicMeshVertex));
			RHIUnlockVertexBuffer(VertexBufferRHI);
		}
	}
};

class FResourceArrayVertexBuffer : public FVertexBuffer
{
public:
	FMeshVertexResourceArray Vertices;
	int32 const NumElements;
	int32 const BufferUsage;

	FResourceArrayVertexBuffer(FMeshVertexArray const& InVertices,
							   int32 const InBufferUsage = BUF_Static)
		: Vertices(true), NumElements(InVertices.Num()), BufferUsage(InBufferUsage)
	{
		Vertices = InVertices;
	}

	virtual void InitRHI() override
	{
		checkSlow(NumElements > 0);
		FRHIResourceCreateInfo CreateInfo(&Vertices);
		VertexBufferRHI = RHICreateVertexBuffer(NumElements * sizeof(FDynamicMeshVertex),
												BufferUsage, CreateInfo);
	}
};

class FSimpleIndexBuffer : public FIndexBuffer
{
public:
	FMeshIndexArray Indices;
	int32 const NumElements;
	int32 const BufferUsage;

	FSimpleIndexBuffer(FMeshIndexArray const& InIndices, int32 const InBufferUsage = BUF_Static)
		: Indices(InIndices), NumElements(InIndices.Num()), BufferUsage(InBufferUsage)
	{
	}

	FSimpleIndexBuffer(int32 const InNumElements, FMeshIndexArray const& InIndices,
					   int32 const InBufferUsage = BUF_Static)
		: Indices(InIndices), NumElements(InNumElements), BufferUsage(InBufferUsage)
	{
	}

	virtual void InitRHI() override
	{
		checkSlow(NumElements > 0);
		FRHIResourceCreateInfo CreateInfo;
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(int32), NumElements * sizeof(int32),
											  BufferUsage, CreateInfo);
		BufferData();
	}

	void BufferData()
	{
		if (Indices.Num() > 0)
		{
			void* Buffer =
				RHILockIndexBuffer(IndexBufferRHI, 0, Indices.Num() * sizeof(int32), RLM_WriteOnly);
			FMemory::Memcpy(Buffer, Indices.GetData(), Indices.Num() * sizeof(int32));
			RHIUnlockIndexBuffer(IndexBufferRHI);
		}
	}
};

class FResourceArrayIndexBuffer : public FIndexBuffer
{
public:
	FMeshIndexResourceArray Indices;
	int32 const NumElements;
	int32 const BufferUsage;

	FResourceArrayIndexBuffer(FMeshIndexArray const& InIndices,
							  int32 const InBufferUsage = BUF_Static)
		: Indices(true), NumElements(InIndices.Num()), BufferUsage(InBufferUsage)
	{
		Indices = InIndices;
	}

	virtual void InitRHI() override
	{
		checkSlow(NumElements > 0);
		FRHIResourceCreateInfo CreateInfo(&Indices);
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(int32), NumElements * sizeof(int32),
											  BufferUsage, CreateInfo);
	}
};

class FMeshVertexFactory : public FLocalVertexFactory
{
public:
	FMeshVertexFactory()
	{
	}

	void Init_RenderThread(const FVertexBuffer* VertexBuffer)
	{
		FDataType NewData;
		NewData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(
			VertexBuffer, FDynamicMeshVertex, Position, VET_Float3);
		NewData.ColorComponent =
			STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Color, VET_Color);
		NewData.TextureCoordinates.Emplace(FVertexStreamComponent(
			VertexBuffer, STRUCT_OFFSET(FDynamicMeshVertex, TextureCoordinate),
			sizeof(FDynamicMeshVertex), VET_Float2));
		NewData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(
			VertexBuffer, FDynamicMeshVertex, TangentX, VET_PackedNormal);
		NewData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(
			VertexBuffer, FDynamicMeshVertex, TangentZ, VET_PackedNormal);
		SetData(NewData);
	}

	void Init(FVertexBuffer* VertexBuffer)
	{
		if (IsInRenderingThread())
		{
			Init_RenderThread(VertexBuffer);
		}
		else
		{
			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
				InitMeshVertexFactory, FMeshVertexFactory*, VertexFactory, this,
				const FVertexBuffer*, VertexBuffer, VertexBuffer,
				{ VertexFactory->Init_RenderThread(VertexBuffer); });
		}
	}
};

class FMeshSceneProxy : public FPrimitiveSceneProxy
{
public:
	int32 const BufferUsage;

	FResourceArrayVertexBuffer VertexBuffer;
	FResourceArrayIndexBuffer IndexBuffer;
	FMeshVertexFactory VertexFactory;

	UMaterialInterface* Material;
	FMaterialRelevance MaterialRelevance;
	TUniformBufferRef<FPrimitiveUniformShaderParameters> PrimitiveUniformBuffer;

public:
	virtual ~FMeshSceneProxy()
	{
		VertexBuffer.ReleaseResource();
		IndexBuffer.ReleaseResource();
		VertexFactory.ReleaseResource();
	}

	FMeshSceneProxy(class UMeshComponent* Component, FMeshVertexArray const& Vertices,
					FMeshIndexArray const& Indices, int32 const InBufferUsage)
		: FPrimitiveSceneProxy(Component),
		  BufferUsage(InBufferUsage),
		  VertexBuffer(Vertices, BufferUsage),
		  IndexBuffer(Indices, BufferUsage),
		  Material(Component->GetMaterial(0)),
		  MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
	{
		Init();
	}

	void Init()
	{
		VertexFactory.Init(&VertexBuffer);

		BeginInitResource(&VertexBuffer);
		BeginInitResource(&IndexBuffer);
		BeginInitResource(&VertexFactory);

		if (Material == nullptr)
		{
			Material = UMaterial::GetDefaultMaterial(MD_Surface);
		}
	}

	virtual void OnTransformChanged() override
	{
		PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(
			GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());
	}

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

	uint32 GetAllocatedSize() const
	{
		return FPrimitiveSceneProxy::GetAllocatedSize();
	}

	virtual uint32 GetMemoryFootprint() const override
	{
		return sizeof(*this) + GetAllocatedSize();
	}
};
