// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "Engine.h"

template <ENamedThreads::Type DesiredThread = ENamedThreads::AnyThread,
		  ESubsequentsMode::Type SubsequentsMode = ESubsequentsMode::TrackSubsequents>
struct FKWTask
{
	TFunction<void()> Function;

	static ENamedThreads::Type GetDesiredThread()
	{
		return DesiredThread;
	}

	static ESubsequentsMode::Type GetSubsequentsMode()
	{
		return SubsequentsMode;
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FKWTask, STATGROUP_TaskGraphTasks);
	}

	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
	{
		Function();
	}

	static FGraphEventRef ConstructAndDispatchWhenReady(TFunction<void()>&& F)
	{
		return TGraphTask<FKWTask>::CreateTask().ConstructAndDispatchWhenReady(FKWTask{F});
	}
};
