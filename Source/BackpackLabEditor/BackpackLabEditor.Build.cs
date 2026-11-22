using UnrealBuildTool;
public class BackpackLabEditor : ModuleRules {
 public BackpackLabEditor(ReadOnlyTargetRules Target):base(Target) {
 PCHUsage=PCHUsageMode.UseExplicitOrSharedPCHs;
 PrivateDependencyModuleNames.AddRange(new[]{"Core","CoreUObject","Engine","BackpackLab","UMG","UMGEditor","UnrealEd","BlueprintGraph","KismetCompiler","Kismet","AssetRegistry","Slate","SlateCore"});
 }
}
