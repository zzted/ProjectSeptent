// ZZ

using UnrealBuildTool;
using System.Collections.Generic;

public class SeptentTarget : TargetRules
{
	public SeptentTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;

		ExtraModuleNames.AddRange( new string[] { "Septent" } );
	}
}
