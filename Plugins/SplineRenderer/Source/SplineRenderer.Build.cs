using System;
using System.IO;

namespace UnrealBuildTool.Rules
{
    public class SplineRenderer : ModuleRules
    {
        private string ThirdPartyPath
        {
            get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty/")); }
        }

        public SplineRenderer(ReadOnlyTargetRules Target)
        {
            PublicIncludePaths.AddRange(new string[] {
                });

            PrivateIncludePaths.AddRange(new string[] {
                    "SplineRenderer/Private",
                });

            PublicDependencyModuleNames.AddRange(new string[] {
                    "Core",
					"CoreUObject",
                    "Engine",
                    "RenderCore",
                    "ShaderCore",
                    "RHI"
                });

            PrivateDependencyModuleNames.AddRange(new string[] {
                });

            DynamicallyLoadedModuleNames.AddRange(new string[] {
                });
        }
    }
}
