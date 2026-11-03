using UnrealBuildTool;
using System.Collections.Generic;

public class BugHuntUETarget : TargetRules
{
	public BugHuntUETarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("BugHuntUE");
	}
}
