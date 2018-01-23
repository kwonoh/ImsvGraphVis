namespace UnrealBuildTool.Rules
{
    public class SplineRenderer : ModuleRules
    {
        public SplineRenderer(ReadOnlyTargetRules Target) : base(Target)
        {
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
        }
    }
}
