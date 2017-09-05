// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "IGVGameMode.h"

#include "IGVPawn.h"
#include "IGVPlayerController.h"

AIGVGameMode::AIGVGameMode() : Super()
{
	PlayerControllerClass = AIGVPlayerController::StaticClass();
	DefaultPawnClass = AIGVPawn::StaticClass();
}
