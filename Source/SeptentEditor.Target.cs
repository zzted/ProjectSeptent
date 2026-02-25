// ZZ

using UnrealBuildTool;
using System.Collections.Generic;

public class SeptentEditorTarget : TargetRules
{
	public SeptentEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V4;

		ExtraModuleNames.AddRange( new string[] { "Septent" } );
	}
}
