// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "IGVEdgeMeshData.h"

FIGVEdgeMeshData::FIGVEdgeMeshData()
{
	VertexBufferOffset[EIGVEdgeRenderGroup::Default] = 0;
	VertexBufferSize[EIGVEdgeRenderGroup::Default] = 0;
	VertexBufferOffset[EIGVEdgeRenderGroup::Highlighted] = 0;
	VertexBufferSize[EIGVEdgeRenderGroup::Highlighted] = 0;
	VertexBufferOffset[EIGVEdgeRenderGroup::Remained] = 0;
	VertexBufferSize[EIGVEdgeRenderGroup::Remained] = 0;

	IndexBufferOffset[EIGVEdgeRenderGroup::Default] = 0;
	IndexBufferSize[EIGVEdgeRenderGroup::Default] = 0;
	IndexBufferOffset[EIGVEdgeRenderGroup::Highlighted] = 0;
	IndexBufferSize[EIGVEdgeRenderGroup::Highlighted] = 0;
	IndexBufferOffset[EIGVEdgeRenderGroup::Remained] = 0;
	IndexBufferSize[EIGVEdgeRenderGroup::Remained] = 0;
}
