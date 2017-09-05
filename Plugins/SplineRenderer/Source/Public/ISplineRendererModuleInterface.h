// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "ModuleManager.h"

class SPLINERENDERER_API ISplineRendererModuleInterface : public IModuleInterface
{
public:
	static inline ISplineRendererModuleInterface& Get()
	{
		return FModuleManager::LoadModuleChecked<ISplineRendererModuleInterface>("SplineRenderer");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("SplineRenderer");
	}
};
