#include "SplineRendererPrivatePCH.h"

class FSplineRendererModule : public ISplineRendererModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FSplineRendererModule, SplineRenderer)

void FSplineRendererModule::StartupModule()
{
}

void FSplineRendererModule::ShutdownModule()
{
}
