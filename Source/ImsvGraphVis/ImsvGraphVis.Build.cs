// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

using UnrealBuildTool;

public class ImsvGraphVis : ModuleRules
{
	public ImsvGraphVis(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "DesktopPlatform",
            "Http",
            "Json",
            "JsonUtilities",
			"RenderCore",
			"Renderer",
			"RHI",
			"ShaderCore",
			"SplineRenderer",
			"Slate",
			"SlateCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
