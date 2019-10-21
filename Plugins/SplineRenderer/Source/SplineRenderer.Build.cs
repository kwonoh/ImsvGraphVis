using System.IO;

namespace UnrealBuildTool.Rules
{
    public class SplineRenderer : ModuleRules
    {
        public SplineRenderer(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

            PublicDependencyModuleNames.AddRange(new string[] {
                "Core",
    			"CoreUObject",
                "Engine",
                "RenderCore",
                "RHI"
            });

            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        }
    }
}
