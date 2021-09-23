// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.IO;

public class SRanipal : ModuleRules
{
    public string GetUProjectPath()
    {
        return Directory.GetParent(ModuleDirectory).Parent.Parent.ToString();
    }

    public SRanipal(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string BaseDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
            string SRanipalDirectory = Path.Combine(BaseDirectory, "Binaries/Win64");

            // Add the import library
            PublicLibraryPaths.Add(SRanipalDirectory);
            PublicAdditionalLibraries.Add(SRanipalDirectory + "/SRanipal.lib");

            // Delay-load the DLL, so we can load it from the right place first
            PublicDelayLoadDLLs.Add("SRanipal.dll");
            string[] dlls = Directory.GetFiles(SRanipalDirectory, "*.dll");
            for (int i = 0; i < dlls.Length; i++)
            {
                if (dlls[i] == "SRanipal.dll") continue;
                RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(SRanipalDirectory, dlls[i])));
            }
        }
        
        PublicIncludePaths.AddRange(
			new string[] {
                Path.GetFullPath(Path.Combine(ModuleDirectory, ".."))+"/SRanipal/Public"
                // ... add public include paths required here ...
			}
            );
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
                ModuleDirectory+"/Private",
                ModuleDirectory+"/Public/Eye",
                ModuleDirectory+"/Public/Lip",
                Path.GetFullPath(Path.Combine(ModuleDirectory, ".."))+"/SRanipalEye/Public",
                Path.GetFullPath(Path.Combine(ModuleDirectory, ".."))+"/SRanipalLip/Public"
                // ... add other private include paths required here ...
            }
            );
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
                //"SRanipalEye",
                //"SRanipalLip"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "Projects",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
