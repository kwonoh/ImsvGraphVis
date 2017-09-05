// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "IGVFunctionLibrary.generated.h"

UCLASS()
class IMSVGRAPHVIS_API UIGVFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	static class AIGVPlayerController* GetPlayerController(UObject const* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	static class AIGVPawn* GetPawn(UObject const* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = ImmersiveGraphVisualization)
	static class AIGVGraphActor* GetGraphActor(UObject const* WorldContextObject);
};
