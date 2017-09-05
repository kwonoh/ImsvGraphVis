// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "IGVPlayerController.h"

#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "GenericPlatformFile.h"
#include "PlatformFilemanager.h"

#include "IGVData.h"
#include "IGVGraphActor.h"
#include "IGVLog.h"

AIGVPlayerController::AIGVPlayerController()
{
	GraphActor = nullptr;
}

void AIGVPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction(TEXT("OpenFile"), IE_Released, this,
							   &AIGVPlayerController::IGV_OpenFile);
}

void AIGVPlayerController::IGV_OpenFile()
{
	if (GraphActor == nullptr)
	{
		IGV_LOG_S(Warning, TEXT("Unable to find GraphActor instance"));
		return;
	}

	UIGVData::OpenFile(GraphActor);
}

void AIGVPlayerController::IGV_SetFieldOfView(float Value)
{
	if (GraphActor == nullptr)
	{
		IGV_LOG_S(Warning, TEXT("Unable to find GraphActor instance"));
		return;
	}

	if (Value < 60)
	{
		IGV_LOG_S(Warning, TEXT("Input value is too small. Minimum FOV should be 60 deg."));
		Value = 60;
	}

	if (Value > 180)
	{
		IGV_LOG_S(Warning, TEXT("Input value is too large. Maximum FOV should be 180 deg."));
		Value = 180;
	}

	GraphActor->FieldOfView = Value;
	GraphActor->UpdateTreemapLayout();
}

void AIGVPlayerController::IGV_SetAspectRatio(float Value)
{
	if (GraphActor == nullptr)
	{
		IGV_LOG_S(Warning, TEXT("Unable to find GraphActor instance"));
		return;
	}

	if (Value < 1)
	{
		IGV_LOG_S(Warning, TEXT("Input value is too small. Minimum aspect ratio should be 1."));
		Value = 1;
	}

	if (Value > 21.0 / 9.0)
	{
		IGV_LOG_S(Warning,
				  TEXT("Input value is too large. Maximum aspect ratio should be 21.0/9.0"));
		Value = 21.0 / 9.0;
	}

	GraphActor->AspectRatio = Value;
	GraphActor->UpdateTreemapLayout();
}

void AIGVPlayerController::IGV_SetTreemapNesting(float Value)
{
	if (GraphActor == nullptr)
	{
		IGV_LOG_S(Warning, TEXT("Unable to find GraphActor instance"));
		return;
	}

	if (Value < 0)
	{
		IGV_LOG_S(Warning, TEXT("Input value is too small. Minimum aspect ratio is 0."));
		Value = 0;
	}

	if (Value > .5)
	{
		IGV_LOG_S(Warning, TEXT("Input value is too large. Maximum aspect ratio should be 0.5"));
		Value = .5;
	}

	GraphActor->TreemapNesting = Value;
	GraphActor->UpdateTreemapLayout();
}

void AIGVPlayerController::IGV_SetHalo(bool const Value)
{
	if (GraphActor == nullptr)
	{
		IGV_LOG_S(Warning, TEXT("Unable to find GraphActor instance"));
		return;
	}

	GraphActor->SetHalo(Value);
}
