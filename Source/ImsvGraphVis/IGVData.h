// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "Core.h"
#include "JsonUtilities.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "IGVData.generated.h"

UCLASS()
class IMSVGRAPHVIS_API UIGVData : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	static FString DefaultDataDirPath();

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	static void OpenFile(class AIGVGraphActor* const GraphActor);

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	static void LoadFile(FString const& FilePath, class AIGVGraphActor* const GraphActor);

	static void DeserializeGraph(TSharedPtr<FJsonObject> GraphJsonObj,
								 class AIGVGraphActor* const GraphActor);

private:
	static void DeserializeNodes(TArray<TSharedPtr<FJsonValue>> const& NodeJsonObjs,
								 class AIGVGraphActor* const GraphActor);

	static void DeserializeEdges(TArray<TSharedPtr<FJsonValue>> const& EdgeJsonObjs,
								 class AIGVGraphActor* const GraphActor);

	static void DeserializeClusters(TArray<TSharedPtr<FJsonValue>> const& ClusterJsonObjs,
									class AIGVGraphActor* const GraphActor);
};
