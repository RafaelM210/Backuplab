using UnrealBuildTool;
public class BackpackLabEditorTarget : TargetRules {
 public BackpackLabEditorTarget(TargetInfo Target) : base(Target) {
 Type = TargetType.Editor; DefaultBuildSettings = BuildSettingsVersion.V7;
 IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
 ExtraModuleNames.AddRange(new[] {"BackpackLab", "BackpackLabEditor"});
 }
}

