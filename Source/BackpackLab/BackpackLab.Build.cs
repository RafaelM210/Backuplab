using UnrealBuildTool;
public class BackpackLab : ModuleRules {
 public BackpackLab(ReadOnlyTargetRules Target) : base(Target) {
 PublicIncludePaths.Add(ModuleDirectory); PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
 PublicDependencyModuleNames.AddRange(new[] {"Core", "CoreUObject", "Engine", "InputCore", "UMG", "Slate", "SlateCore"});
 }
}

