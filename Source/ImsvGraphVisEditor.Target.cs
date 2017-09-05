// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ImsvGraphVisEditorTarget : TargetRules
{
	public ImsvGraphVisEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange( new string[] { "ImsvGraphVis" } );
	}
}
