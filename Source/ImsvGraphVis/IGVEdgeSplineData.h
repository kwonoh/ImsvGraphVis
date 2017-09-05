// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct IMSVGRAPHVIS_API FIGVEdgeSplineControlPointData
{
	FVector Position;
	float Level;
	float Knot;
};

struct IMSVGRAPHVIS_API FIGVEdgeSplineSegmentData
{
	uint32 SplineIdx;
	uint32 BeginControlPointIdx;
	uint32 NumSamples;
	uint32 MeshVertexBufferOffset;
	uint32 MeshIndexBufferOffset;
};

struct IMSVGRAPHVIS_API FIGVEdgeSplineData
{
	FVector StartPosition;
	FVector EndPosition;
	FVector StartColor_HCL;
	FVector EndColor_HCL;
	float BundlingStrength;
	uint32 BeginControlPointIdx;
	uint32 NumControlPoints;
	uint32 MeshVertexBufferOffset;
};
