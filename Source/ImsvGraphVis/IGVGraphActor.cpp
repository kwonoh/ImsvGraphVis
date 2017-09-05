// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "IGVGraphActor.h"

#include "Components/PostProcessComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SphereComponent.h"

#include "KWColorSpace.h"
#include "KWTask.h"

#include "IGVData.h"
#include "IGVEdgeMeshComponent.h"
#include "IGVFunctionLibrary.h"
#include "IGVLog.h"
#include "IGVNodeActor.h"
#include "IGVPawn.h"
#include "IGVPlayerController.h"
#include "IGVTreemapLayout.h"

UMaterialInterface* GetOutlineMaterial()
{
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(
		TEXT("/Game/Materials/PP_Outline_EdgeDetect.PP_Outline_EdgeDetect"));
	return MaterialAsset.Succeeded() ? MaterialAsset.Object->GetMaterial() : nullptr;
}

AIGVGraphActor::AIGVGraphActor()
	: Filename("lesmis.igv.json"),
	  Nodes(),
	  Edges(),
	  Clusters(),
	  PlanarExtent(1.f, 1.f),
	  FieldOfView(90.f),
	  AspectRatio(16.f / 9.f),
	  ProjectionMode(EIGVProjection::Sphere_Stereographic_RadialWarping),
	  ClusterLevelScale(1.f),
	  ClusterLevelExponent(2.f),
	  ClusterLevelOffset(.1f),
	  TreemapNesting(.1f),
	  EdgeSplineResolution(24),
	  EdgeWidth(8.f),
	  EdgeNumSides(4),
	  EdgeBundlingStrength(.9f),
	  ColorHueMin(0.f),
	  ColorHueMax(210.f),
	  ColorHueOffset(0.f),
	  ColorChroma(.5f),
	  ColorLuminance(.5f),
	  PickRayDistSortedNodes(),
	  LastNearestNode(nullptr),
	  LastPickedNode(nullptr),
	  PickDistanceThreshold(30),
	  DefaultLevelScale(1.f),
	  HighlightedLevelScale(.5f),
	  NeighborHighlightedLevelScale(.75f),
	  bUpdateDefaultEdgeMeshRequired(true)
{
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	RootComponent = SphereComponent;

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	PostProcessComponent->AttachToComponent(RootComponent,
											FAttachmentTransformRules::KeepRelativeTransform);

	SkyLightComponent = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLightComponent->AttachToComponent(RootComponent,
										 FAttachmentTransformRules::KeepRelativeTransform);
	SkyLightComponent->SourceType = ESkyLightSourceType::SLS_SpecifiedCubemap;
	SkyLightComponent->SetIntensity(3.75);
	SkyLightComponent->SetCastShadows(false);

	DefaultEdgeGroupMeshComponent =
		CreateDefaultSubobject<UIGVEdgeMeshComponent>(TEXT("DefaultEdgeGroupMeshComponent"));
	DefaultEdgeGroupMeshComponent->Init(this, EIGVEdgeRenderGroup::Default);
	DefaultEdgeGroupMeshComponent->AttachToComponent(
		RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	HighlightedEdgeGroupMeshComponent =
		CreateDefaultSubobject<UIGVEdgeMeshComponent>(TEXT("HighlightedEdgeGroupMeshComponent"));
	HighlightedEdgeGroupMeshComponent->Init(this, EIGVEdgeRenderGroup::Highlighted);
	HighlightedEdgeGroupMeshComponent->AttachToComponent(
		RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	RemainedEdgeGroupMeshComponent =
		CreateDefaultSubobject<UIGVEdgeMeshComponent>(TEXT("RemainedEdgeGroupMeshComponent"));
	RemainedEdgeGroupMeshComponent->Init(this, EIGVEdgeRenderGroup::Remained);
	RemainedEdgeGroupMeshComponent->AttachToComponent(
		RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	SetSphereRadius(1000.0f);
	ResetAmbientOcclusion();

	GetOutlineMaterial();
}

void AIGVGraphActor::BeginPlay()
{
	Super::BeginPlay();

	AIGVPlayerController* const PlayerController = UIGVFunctionLibrary::GetPlayerController(this);
	if (PlayerController)
	{
		PlayerController->GraphActor = this;
	}
	else
	{
		IGV_LOG_S(Warning, TEXT("Unable to find PlayerController"));
	}

	AIGVPawn* const Pawn = UIGVFunctionLibrary::GetPawn(this);
	if (Pawn)
	{
		Pawn->GraphActor = this;
	}
	else
	{
		IGV_LOG_S(Warning, TEXT("Unable to find Pawn"));
	}

	OutlineMaterialInstance = UMaterialInstanceDynamic::Create(GetOutlineMaterial(), this);

	if (!Filename.IsEmpty())
	{
		UIGVData::LoadFile(FPaths::Combine(UIGVData::DefaultDataDirPath(), Filename), this);
	}
}

void AIGVGraphActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateInteraction();
	UpdateEdgeMeshes();
}

void AIGVGraphActor::EmptyGraph()
{
	for (AIGVNodeActor* Node : Nodes)
	{
		Node->Destroy();
	}
	Nodes.Empty();
	Edges.Empty();
	Clusters.Empty();

	PickRayDistSortedNodes.Empty();
	LastNearestNode = nullptr;
	LastPickedNode = nullptr;
}

void AIGVGraphActor::SetupGraph()
{
	SetupNodes();
	SetupEdges();
	SetupClusters();
	UpdateColors();
	UpdateTreemapLayout();
	SetupEdgeMeshes();
}

void AIGVGraphActor::SetupNodes()
{
	for (AIGVNodeActor* const Node : Nodes)
	{
		Node->SetText(Node->Label);
		PickRayDistSortedNodes.Add(Node);

		IGV_LOG(Log, TEXT("Node: %s"), *Node->ToString());
	}
}

void AIGVGraphActor::SetupEdges()
{
	for (FIGVEdge& Edge : Edges)
	{
		Edge.SourceNode = Nodes[Edge.SourceIdx];
		Edge.TargetNode = Nodes[Edge.TargetIdx];

		Edge.SourceNode->Edges.Add(&Edge);
		Edge.TargetNode->Edges.Add(&Edge);

		Edge.SourceNode->Neighbors.Add(Edge.TargetNode);
		Edge.TargetNode->Neighbors.Add(Edge.SourceNode);

		IGV_LOG(Log, TEXT("Edge: %s"), *Edge.ToString());
	}
}

void AIGVGraphActor::SetupClusters()
{
	RootCluster = &Clusters.Last();
	check(RootCluster->ParentIdx == -1);

	for (FIGVCluster& Cluster : Clusters)
	{
		if (!Cluster.IsRoot())
		{
			Cluster.Parent = &Clusters[Cluster.ParentIdx];
			Cluster.Parent->Children.Add(&Cluster);
		}

		if (Cluster.IsLeaf())
		{
			Cluster.Node = Nodes[Cluster.NodeIdx];
		}
	}

	RootCluster->SetNumDescendantNodes();

	for (FIGVCluster& Cluster : Clusters)
	{
		IGV_LOG(Log, TEXT("Cluster: %s"), *Cluster.ToString());
	}

	for (FIGVEdge& Edge : Edges)
	{
		Edge.SetupClusters();
	}
}

float AIGVGraphActor::GetSphereRadius() const
{
	return SphereComponent->GetUnscaledSphereRadius();
}

void AIGVGraphActor::SetSphereRadius(float Radius)
{
	SphereComponent->InitSphereRadius(Radius);
}

FVector AIGVGraphActor::Project(FVector2D const& P) const
{
	switch (ProjectionMode)
	{
		case EIGVProjection::Sphere_SphericalCoordinates:
			return UIGVProjection::ToSphere_SphericalCoordinates(P);
		case EIGVProjection::Sphere_Gnomonic:  //
			return UIGVProjection::ToSphere_Gnomonic(P);
		case EIGVProjection::Sphere_Gnomonic_RadialWarping:
			return UIGVProjection::ToSphere_Gnomonic_RadialWarping(P);
		case EIGVProjection::Sphere_Gnomonic_IndependentWarping:
			return UIGVProjection::ToSphere_Gnomonic_IndependentWarping(P);
		case EIGVProjection::Sphere_Stereographic:  //
			return UIGVProjection::ToSphere_Stereographic(P);
		case EIGVProjection::Sphere_Stereographic_RadialWarping:
			return UIGVProjection::ToSphere_Stereographic_RadialWarping(P);
		case EIGVProjection::Sphere_Stereographic_IndependentWarping:
			return UIGVProjection::ToSphere_Stereographic_IndependentWarping(P);
		default: checkNoEntry(); break;
	}
	return FVector::ZeroVector;
}

void AIGVGraphActor::UpdatePlanarExtent()
{
	PlanarExtent.X = FMath::DegreesToRadians(FieldOfView * 0.5);
	PlanarExtent.Y = PlanarExtent.X / AspectRatio;
}

void AIGVGraphActor::NormalizeNodePosition()
{
	UpdatePlanarExtent();

	FBox2D Bounds(ForceInitToZero);
	for (AIGVNodeActor* const Node : Nodes)
	{
		Bounds += Node->Pos2D;
	}

	FVector2D const BoundCenter = Bounds.GetCenter();
	FVector2D const BoundExtent = Bounds.GetExtent();

	for (AIGVNodeActor* const Node : Nodes)
	{
		FVector2D& P = Node->Pos2D;
		P -= BoundCenter;
		P /= BoundExtent;
		P *= PlanarExtent;
		Node->SetPos3D();
	}

	RootCluster->SetPosNonLeaf();

	bUpdateDefaultEdgeMeshRequired = true;
}

void AIGVGraphActor::UpdateTreemapLayout()
{
	UpdatePlanarExtent();

	FIGVTreemapLayout Layout(this);
	Layout.Compute();

	NormalizeNodePosition();
}

void AIGVGraphActor::SetupEdgeMeshes()
{
	FGraphEventArray Tasks;
	for (FIGVEdge& Edge : Edges)
	{
		Tasks.Add(FKWTask<>::ConstructAndDispatchWhenReady([&]() {
			// Edge.UpdateRenderGroup();
			Edge.UpdateSplineControlPoints();
		}));
	}
	FTaskGraphInterface::Get().WaitUntilTasksComplete(Tasks);

	DefaultEdgeGroupMeshComponent->Setup();
	bUpdateDefaultEdgeMeshRequired = false;

	HighlightedEdgeGroupMeshComponent->Setup();
	RemainedEdgeGroupMeshComponent->Setup();
}

void AIGVGraphActor::UpdateEdgeMeshes()
{
	EdgeUpdateTasks.Reset();

	if (bUpdateDefaultEdgeMeshRequired)
	{
		for (FIGVEdge& Edge : Edges)
		{
			EdgeUpdateTasks.Add(FKWTask<>::ConstructAndDispatchWhenReady(
				[&]() { Edge.UpdateDefaultSplineControlPoints(); }));
		}
		FTaskGraphInterface::Get().WaitUntilTasksComplete(EdgeUpdateTasks);
		DefaultEdgeGroupMeshComponent->Update();
		bUpdateDefaultEdgeMeshRequired = false;

		EdgeUpdateTasks.Reset();

		for (FIGVEdge& Edge : Edges)
		{
			EdgeUpdateTasks.Add(FKWTask<>::ConstructAndDispatchWhenReady([&]() {
				Edge.UpdateRenderGroup();
				Edge.UpdateSplineControlPoints();
				Edge.bUpdateMeshRequired = false;
			}));
		}
		FTaskGraphInterface::Get().WaitUntilTasksComplete(EdgeUpdateTasks);
		HighlightedEdgeGroupMeshComponent->Update();
		RemainedEdgeGroupMeshComponent->Update();
	}
	else
	{
		for (FIGVEdge& Edge : Edges)
		{
			if (Edge.bUpdateMeshRequired)
			{
				EdgeUpdateTasks.Add(FKWTask<>::ConstructAndDispatchWhenReady([&]() {
					Edge.UpdateRenderGroup();
					Edge.UpdateSplineControlPoints();
					Edge.bUpdateMeshRequired = false;
				}));
			}
		}
		FTaskGraphInterface::Get().WaitUntilTasksComplete(EdgeUpdateTasks);

		if (EdgeUpdateTasks.Num() > 0)
		{
			HighlightedEdgeGroupMeshComponent->Update();
			RemainedEdgeGroupMeshComponent->Update();
		}
	}
}

void AIGVGraphActor::UpdateColors()
{
	int32 const NumNodes = Nodes.Num();
	int32 Idx = 0;

	RootCluster->ForEachDescendantFirst([&](FIGVCluster& Cluster) {
		if (Cluster.IsLeaf())
		{
			float const Alpha = float(Idx) / NumNodes;
			float const Hue = FMath::Lerp(ColorHueMin, ColorHueMax, Alpha);
			Cluster.Node->SetColor(
				FLinearColor(UKWColorSpace::HCLtoRGB(Hue / 360.0, ColorChroma, ColorLuminance)));
			Idx++;
		}
	});

	bUpdateDefaultEdgeMeshRequired = true;
}

void AIGVGraphActor::ResetAmbientOcclusion()
{
	FPostProcessSettings& PostProcessSettings = PostProcessComponent->Settings;

	PostProcessSettings.bOverride_AmbientOcclusionIntensity = true;
	PostProcessSettings.AmbientOcclusionIntensity = 1.0;  // 0.5

	PostProcessSettings.bOverride_AmbientOcclusionRadius = true;
	PostProcessSettings.AmbientOcclusionRadius = 500.0;  // 40.0

	PostProcessSettings.bOverride_AmbientOcclusionRadiusInWS = true;
	PostProcessSettings.AmbientOcclusionRadiusInWS = true;  // false

	PostProcessSettings.bOverride_AmbientOcclusionDistance_DEPRECATED = true;
	PostProcessSettings.AmbientOcclusionDistance_DEPRECATED = 10000.0;  // 80.0

	PostProcessSettings.bOverride_AmbientOcclusionPower = true;
	PostProcessSettings.AmbientOcclusionPower = 2.5;  // 2.0

	PostProcessSettings.bOverride_AmbientOcclusionBias = true;
	PostProcessSettings.AmbientOcclusionBias = 0.5;  // 3.0

	PostProcessSettings.bOverride_AmbientOcclusionQuality = true;
	PostProcessSettings.AmbientOcclusionQuality = 100;  // 50.0

	PostProcessSettings.bOverride_AmbientOcclusionMipBlend = false;
	PostProcessSettings.AmbientOcclusionMipBlend = 0.6;  // 0.6

	PostProcessSettings.bOverride_AmbientOcclusionMipScale = false;
	PostProcessSettings.AmbientOcclusionMipScale = 1.0;  // 1.7
}

void AIGVGraphActor::UpdateInteraction()
{
	if (PickRayDistSortedNodes.Num() == 0) return;

	UpdateNodeDistanceToPickRay();
	AIGVNodeActor* const NearestNode = PickRayDistSortedNodes[0];

	// update nearest node
	if (LastNearestNode != nullptr)  // has previous nearest node actor
	{
		if (LastNearestNode != NearestNode)  // different node actor is nearest
		{
			LastNearestNode->EndNearest();
			LastNearestNode = NearestNode;
			LastNearestNode->BeginNearest();
		}
	}
	else
	{
		LastNearestNode = NearestNode;
		LastNearestNode->BeginNearest();
	}

	// update picked node
	if (NearestNode->IsPicked())  // nearest node actor is picked node actor
	{
		if (LastPickedNode != nullptr)  // has previous picked node actor
		{
			if (LastPickedNode != NearestNode)  // different node picked
			{
				LastPickedNode->EndPicked();
				LastPickedNode = NearestNode;
				LastPickedNode->BeginPicked();
			}
		}
		else  // new node actor picked
		{
			LastPickedNode = NearestNode;
			LastPickedNode->BeginPicked();
		}
	}
	else
	{
		if (LastPickedNode != nullptr)
		{
			LastPickedNode->EndPicked();
		}
		LastPickedNode = nullptr;
	}
}

void AIGVGraphActor::UpdateNodeDistanceToPickRay()
{
	AIGVPawn* const Pawn = UIGVFunctionLibrary::GetPawn(this);
	if (Pawn == nullptr) return;

	for (AIGVNodeActor* const Node : Nodes)
	{
		Node->DistanceToPickRay = FMath::PointDistToLine(
			Node->GetActorLocation(), Pawn->PickRayDirection, Pawn->PickRayOrigin);
	}

	PickRayDistSortedNodes.Sort([](AIGVNodeActor const& A, AIGVNodeActor const& B) {
		return A.DistanceToPickRay < B.DistanceToPickRay;
	});
}

void AIGVGraphActor::OnLeftMouseButtonReleased()
{
	if (LastPickedNode != nullptr)
	{
		LastPickedNode->OnLeftMouseButtonReleased();
	}
}

void AIGVGraphActor::SetHalo(bool const bValue)
{
	PostProcessComponent->Settings.RemoveBlendable(OutlineMaterialInstance);

	if (bValue)
	{
		PostProcessComponent->Settings.AddBlendable(OutlineMaterialInstance, 1.0);
	}
}
