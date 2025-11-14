// TFTUnrealDemo.Build.cs
using UnrealBuildTool;

public class TFTUnrealDemo : ModuleRules
{
    public TFTUnrealDemo(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "AIModule",
            "GameplayTasks",
            "NavigationSystem"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}