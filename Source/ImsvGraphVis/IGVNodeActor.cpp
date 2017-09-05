// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "IGVNodeActor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Materials/Material.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"

#include "IGVGraphActor.h"
#include "IGVLog.h"

UMaterialInterface* GetNodeMaterial()
{
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(
		TEXT("/Game/Materials/M_Node.M_Node"));
	return MaterialAsset.Succeeded() ? MaterialAsset.Object->GetMaterial() : nullptr;
}

AIGVNodeActor::AIGVNodeActor()
	: GraphActor(nullptr),
	  Label("Unknown"),
	  Pos2D(FVector2D::ZeroVector),
	  Pos3D(FVector::ZeroVector),
	  LevelScale(1.f),
	  LevelScaleBeforeTransition(1.f),
	  LevelScaleAfterTransition(1.f),
	  Color(FLinearColor::White),
	  DistanceToPickRay(FLT_MAX),
	  bIsHighlighted(false),
	  NumHighlightedNeighbors(0),
	  MeshMaterialInstance(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Node Mesh"));
	RootComponent = MeshComponent;

	TextRenderComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Node Text"));
	TextRenderComponent->AttachToComponent(RootComponent,
										   FAttachmentTransformRules::KeepRelativeTransform);
	TextRenderComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	TextRenderComponent->VerticalAlignment = EVerticalTextAligment::EVRTA_TextTop;
	TextRenderComponent->SetRelativeLocation(FVector(20, 0, -12));
	TextRenderComponent->SetWorldSize(20);
	TextRenderComponent->SetVisibility(false);

	GetNodeMaterial();
}

void AIGVNodeActor::Init(AIGVGraphActor* const InGraphActor)
{
	GraphActor = InGraphActor;

	MeshMaterialInstance = UMaterialInstanceDynamic::Create(GetNodeMaterial(), this);
	if (MeshMaterialInstance) MeshComponent->SetMaterial(0, MeshMaterialInstance);

	LevelScale = LevelScaleBeforeTransition = LevelScaleAfterTransition =
		GraphActor->DefaultLevelScale;
}

void AIGVNodeActor::BeginPlay()
{
	Super::BeginPlay();
}

void AIGVNodeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FString AIGVNodeActor::ToString() const
{
	TArray<FString> AncStrs;
	for (int32 const ClusterIdx : AncIdxs)
	{
		AncStrs.Add(FString::FromInt(ClusterIdx));
	}

	return FString::Printf(TEXT("Idx=%d Label=%s Pos2D=(%s) Pos3D=(%s) Ancestors=[%s]"), Idx,
						   *Label, *Pos2D.ToString(), *Pos3D.ToString(),
						   *FString::Join(AncStrs, TEXT(" ")));
}

void AIGVNodeActor::SetPos3D()
{
	Pos3D = GraphActor->Project(Pos2D);
	RootComponent->SetRelativeLocation(Pos3D * LevelScale * GraphActor->GetSphereRadius());
	UpdateRotation();
}

void AIGVNodeActor::SetColor(FLinearColor const& C)
{
	Color = C;
	if (MeshMaterialInstance != nullptr)
		MeshMaterialInstance->SetVectorParameterValue(TEXT("Base Color"), Color);
}

void AIGVNodeActor::SetHalo(bool const bValue)
{
	MeshComponent->SetRenderCustomDepth(bValue);
}

void AIGVNodeActor::SetText(FString const& Value)
{
	TextRenderComponent->SetText(FText::FromString(Value));
}

void AIGVNodeActor::UpdateRotation()
{
	RootComponent->SetRelativeRotation((FVector::ZeroVector - Pos3D).Rotation());
}

bool AIGVNodeActor::IsPicked() const
{
	return DistanceToPickRay < GraphActor->PickDistanceThreshold;
}

void AIGVNodeActor::BeginNearest()
{
}

void AIGVNodeActor::EndNearest()
{
}

void AIGVNodeActor::BeginPicked()
{
	TextRenderComponent->SetVisibility(true);
}

void AIGVNodeActor::EndPicked()
{
	if (!bIsHighlighted) TextRenderComponent->SetVisibility(false);
}

void AIGVNodeActor::BeginHighlighted()
{
	bIsHighlighted = true;
	SetHalo(true);
	TextRenderComponent->SetVisibility(true);

	LevelScaleAfterTransition = GraphActor->HighlightedLevelScale;
	BeginTransition();

	for (AIGVNodeActor* const Neighbor : Neighbors)
	{
		Neighbor->BeginNeighborHighlighted();
	}
}

void AIGVNodeActor::EndHighlighted()
{
	bIsHighlighted = false;
	TextRenderComponent->SetVisibility(false);

	if (HasHighlightedNeighbor())
	{
		LevelScaleAfterTransition = GraphActor->NeighborHighlightedLevelScale;
		SetHalo(true);
	}
	else
	{
		LevelScaleAfterTransition = GraphActor->DefaultLevelScale;
		SetHalo(false);
	}

	BeginTransition();

	for (AIGVNodeActor* const Neighbor : Neighbors)
	{
		Neighbor->EndNeighborHighlighted();
	}
}

void AIGVNodeActor::BeginNeighborHighlighted()
{
	bool const TransionRequired = !(bIsHighlighted || HasHighlightedNeighbor());

	NumHighlightedNeighbors++;

	if (TransionRequired)
	{
		LevelScaleAfterTransition = GraphActor->NeighborHighlightedLevelScale;
		BeginTransition();
		SetHalo(true);
	}
}

void AIGVNodeActor::EndNeighborHighlighted()
{
	NumHighlightedNeighbors--;
	check(NumHighlightedNeighbors >= 0);

	bool const TransionRequired = !(bIsHighlighted || HasHighlightedNeighbor());

	if (TransionRequired)
	{
		LevelScaleAfterTransition = GraphActor->DefaultLevelScale;
		BeginTransition();
		SetHalo(false);
	}
}

bool AIGVNodeActor::HasHighlightedNeighbor() const
{
	return NumHighlightedNeighbors > 0;
}

void AIGVNodeActor::BeginTransition()
{
	LevelScaleBeforeTransition = LevelScale;

	for (FIGVEdge* const Edge : Edges)
	{
		Edge->BeginTransition();
	}

	PlayFromStartHighlightTransitionTimeline();
}

void AIGVNodeActor::OnLeftMouseButtonReleased()
{
	check(IsPicked());

	if (bIsHighlighted)
	{
		EndHighlighted();
	}
	else
	{
		BeginHighlighted();
	}
}

void AIGVNodeActor::OnHighlightTransitionTimelineUpdate(ETimelineDirection::Type const Direction,
														float const Alpha)
{
	LevelScale = FMath::Lerp(LevelScaleBeforeTransition, LevelScaleAfterTransition, Alpha);
	SetPos3D();

	for (FIGVEdge* const Edge : Edges)
	{
		Edge->OnHighlightTransitionTimelineUpdate(Direction, Alpha);
	}
}

void AIGVNodeActor::OnHighlightTransitionTimelineFinished(ETimelineDirection::Type const Direction)
{
	for (FIGVEdge* const Edge : Edges)
	{
		Edge->OnHighlightTransitionTimelineFinished(Direction);
	}
}
