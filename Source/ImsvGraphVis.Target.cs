// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ImsvGraphVisTarget : TargetRules
{
	public ImsvGraphVisTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange( new string[] { "ImsvGraphVis" } );
	}
}
