#include "SplineRendererPrivatePCH.h"

class FSplineRendererModule : public ISplineRendererModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FSplineRendererModule, SplineRenderer)

void FSplineRendererModule::StartupModule()
{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 21
	FString ShaderDirectory = FPaths::Combine(
				FPaths::ProjectDir(), TEXT("Plugins"), TEXT("SplineRenderer"), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping("/Plugin/SplineRenderer", ShaderDirectory);
#endif
}

void FSplineRendererModule::ShutdownModule()
{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 21
	ResetAllShaderSourceDirectoryMappings();
#endif
}
