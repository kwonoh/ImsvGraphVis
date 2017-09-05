// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "IGVData.h"

#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "GenericPlatformFile.h"
#include "PlatformFilemanager.h"

#include "KWJson.h"

#include "IGVGraphActor.h"
#include "IGVLog.h"
#include "IGVNodeActor.h"

FString UIGVData::DefaultDataDirPath()
{
	return FPaths::Combine(*FPaths::GameSavedDir(), TEXT("Data/Graph/"));
}

void UIGVData::OpenFile(AIGVGraphActor* const GraphActor)
{
	IDesktopPlatform* const DesktopPlatform = FDesktopPlatformModule::Get();
	FString const DefaultPath = DefaultDataDirPath();

	void const* ParentWindowHandle = nullptr;
	UGameViewportClient* GameVieport = GEngine->GameViewport;
	if (GameVieport)
	{
		TSharedPtr<SWindow> Window = GameVieport->GetWindow();
		if (Window.IsValid())
		{
			TSharedPtr<FGenericWindow> NativeWindow = Window->GetNativeWindow();
			if (NativeWindow.IsValid())
			{
				ParentWindowHandle = NativeWindow->GetOSWindowHandle();
			}
		}
	}

	TArray<FString> SelectedFiles;
	if (DesktopPlatform->OpenFileDialog(ParentWindowHandle,		 // ParentWindowHandle
										TEXT("Open Data File"),  // DialogTitle
										DefaultPath,			 // DefaultPath
										TEXT(""),				 // DefaultFile
										TEXT(""),				 // FileTypes
										EFileDialogFlags::None,  // Flags
										SelectedFiles))
	{
		check(SelectedFiles.Num() == 1);
		LoadFile(SelectedFiles[0], GraphActor);
	}
}

void UIGVData::LoadFile(FString const& FilePath, AIGVGraphActor* const GraphActor)
{
	if (!FPaths::FileExists(FilePath))
	{
		IGV_LOG_S(Error, TEXT("File not exist: %s"), *FilePath);
		return;
	}

	FString const Filename = FPaths::GetCleanFilename(FilePath);
	IGV_LOG_S(Log, TEXT("Load File: %s"), *Filename);

	FString JsonStr;
	FFileHelper::LoadFileToString(JsonStr, *FilePath);

	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject());
	if (!DeserializeJson(JsonStr, JsonObj))
	{
		IGV_LOG_S(Error, TEXT("Unable to deserialize JSON file: %s"), *JsonStr);
		return;
	}
	DeserializeGraph(JsonObj, GraphActor);
}

void UIGVData::DeserializeGraph(TSharedPtr<FJsonObject> GraphJsonObj,
								AIGVGraphActor* const GraphActor)
{
	if (GraphActor == nullptr)
	{
		IGV_LOG_S(Error, TEXT("Unable to get GraphActor"));
		return;
	}
	GraphActor->EmptyGraph();

	TArray<TSharedPtr<FJsonValue>> const* NodeJsonObjs;
	if (!GraphJsonObj->TryGetArrayField(TEXT("nodes"), NodeJsonObjs))
	{
		IGV_LOG_S(Error, TEXT("Unable to get node json objects"));
		return;
	}

	TArray<TSharedPtr<FJsonValue>> const* EdgeJsonObjs;
	if (!GraphJsonObj->TryGetArrayField(TEXT("links"), EdgeJsonObjs))
	{
		IGV_LOG_S(Error, TEXT("Unable to get edge json objects"));
		return;
	}

	TArray<TSharedPtr<FJsonValue>> const* ClusterJsonObjs;
	if (!GraphJsonObj->TryGetArrayField(TEXT("clusters"), ClusterJsonObjs))
	{
		IGV_LOG_S(Error, TEXT("Unable to get cluster json objects"));
		return;
	}

	DeserializeNodes(*NodeJsonObjs, GraphActor);
	DeserializeEdges(*EdgeJsonObjs, GraphActor);
	DeserializeClusters(*ClusterJsonObjs, GraphActor);

	GraphActor->SetupGraph();
}

void UIGVData::DeserializeNodes(TArray<TSharedPtr<FJsonValue>> const& NodeJsonObjs,
								AIGVGraphActor* const GraphActor)
{
	UWorld* const World = GraphActor->GetWorld();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GraphActor;
	SpawnParams.Instigator = GraphActor->Instigator;

	for (TSharedPtr<FJsonValue> const& JsonVal : NodeJsonObjs)
	{
		TSharedPtr<FJsonObject> const NodeJsonObj = JsonVal->AsObject();

		AIGVNodeActor* const NodeActor = World->SpawnActor<AIGVNodeActor>(
			GraphActor->NodeActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		NodeActor->AttachToActor(GraphActor, FAttachmentTransformRules::KeepRelativeTransform);
		NodeActor->Init(GraphActor);
		GraphActor->Nodes.Add(NodeActor);

		if (!JsonObjectToUObject(NodeJsonObj.ToSharedRef(), NodeActor))
		{
			IGV_LOG_S(Error, TEXT("Unable to deserialize a cluster"));
		}
	}
}

void UIGVData::DeserializeEdges(TArray<TSharedPtr<FJsonValue>> const& EdgeJsonObjs,
								AIGVGraphActor* const GraphActor)
{
	for (TSharedPtr<FJsonValue> const& JsonVal : EdgeJsonObjs)
	{
		TSharedPtr<FJsonObject> const EdgeJsonObj = JsonVal->AsObject();

		GraphActor->Edges.Emplace(GraphActor);
		FIGVEdge& Edge = GraphActor->Edges.Last();

		if (!JsonObjectToUStruct(EdgeJsonObj.ToSharedRef(), &Edge))
		{
			IGV_LOG_S(Error, TEXT("Unable to deserialize an edge"));
		}
	}
}

void UIGVData::DeserializeClusters(TArray<TSharedPtr<FJsonValue>> const& ClusterJsonObjs,
								   AIGVGraphActor* const GraphActor)
{
	for (TSharedPtr<FJsonValue> const& JsonVal : ClusterJsonObjs)
	{
		TSharedPtr<FJsonObject> const ClusterJsonObj = JsonVal->AsObject();

		GraphActor->Clusters.Emplace(GraphActor);
		FIGVCluster& Cluster = GraphActor->Clusters.Last();

		if (!JsonObjectToUStruct(ClusterJsonObj.ToSharedRef(), &Cluster))
		{
			IGV_LOG_S(Error, TEXT("Unable to deserialize a cluster"));
		}
	}
}
