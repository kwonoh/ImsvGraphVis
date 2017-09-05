// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
namespace EIGVEdgeRenderGroup
{
enum Type
{
	Default = 0,
	Highlighted = 1,
	Remained = 2,
	NumGroups = 3
};
}

struct IMSVGRAPHVIS_API FIGVEdgeMeshData
{
	int32 VertexBufferOffset[EIGVEdgeRenderGroup::NumGroups];
	int32 VertexBufferSize[EIGVEdgeRenderGroup::NumGroups];
	int32 IndexBufferOffset[EIGVEdgeRenderGroup::NumGroups];
	int32 IndexBufferSize[EIGVEdgeRenderGroup::NumGroups];

	FIGVEdgeMeshData();
};
