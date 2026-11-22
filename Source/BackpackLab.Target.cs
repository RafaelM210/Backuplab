using UnrealBuildTool;
public class BackpackLabTarget : TargetRules {
 public BackpackLabTarget(TargetInfo Target) : base(Target) {
 Type = TargetType.Game; DefaultBuildSettings = BuildSettingsVersion.V7;
 IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
 ExtraModuleNames.Add("BackpackLab");
 }
}

