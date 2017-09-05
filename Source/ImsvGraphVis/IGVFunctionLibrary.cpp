// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "IGVFunctionLibrary.h"

#include "Engine.h"

#include "IGVGraphActor.h"
#include "IGVPawn.h"
#include "IGVPlayerController.h"

AIGVPlayerController* UIGVFunctionLibrary::GetPlayerController(UObject const* WorldContextObject)
{
	UWorld* const World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	return Cast<AIGVPlayerController>(World->GetFirstPlayerController());
}

AIGVPawn* UIGVFunctionLibrary::GetPawn(UObject const* WorldContextObject)
{
	AIGVPlayerController* const PlayerController = GetPlayerController(WorldContextObject);
	return PlayerController != nullptr ? Cast<AIGVPawn>(PlayerController->GetPawn()) : nullptr;
}

AIGVGraphActor* UIGVFunctionLibrary::GetGraphActor(UObject const* WorldContextObject)
{
	AIGVPlayerController* const PlayerController = GetPlayerController(WorldContextObject);
	return PlayerController != nullptr ? PlayerController->GraphActor : nullptr;
}
