// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "Components/MeshComponent.h"
#include "CoreMinimal.h"

#include "KWMeshElement.h"

#include "IGVEdgeMeshData.h"
#include "IGVEdgeSplineData.h"

#include "IGVEdgeMeshComponent.generated.h"

UCLASS()
class IMSVGRAPHVIS_API UIGVEdgeMeshComponent : public UMeshComponent
{
	GENERATED_BODY()

public:
	class AIGVGraphActor* GraphActor;

	UPROPERTY()
	TEnumAsByte<EIGVEdgeRenderGroup::Type> RenderGroup;

	TArray<FIGVEdgeSplineControlPointData> SplineControlPointData;
	TArray<FIGVEdgeSplineSegmentData> SplineSegmentData;
	TArray<FIGVEdgeSplineData> SplineData;

	int32 NumMeshIndices;
	int32 NumMeshVertices;

	FMeshIndexArray MeshIndices;

	UPROPERTY()
	class UMaterialInstanceDynamic* MaterialInstance;

public:
	UIGVEdgeMeshComponent();

	void Init(class AIGVGraphActor* const InGraphActor,
			  EIGVEdgeRenderGroup::Type const InRenderGroup);

	void Setup();
	void Update();

	class FIGVEdgeMeshSceneProxy* GetSceneProxy() const;

	// Begin USceneComponent interface.
	virtual void SendRenderDynamicData_Concurrent() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	// Begin USceneComponent interface.

	// Begin UPrimitiveComponent interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	// End UPrimitiveComponent interface.

	// Begin UMeshComponent interface.
	virtual int32 GetNumMaterials() const override
	{
		return 1;
	}
	// End UMeshComponent interface.

	void SetHalo(bool const bValue);
};
