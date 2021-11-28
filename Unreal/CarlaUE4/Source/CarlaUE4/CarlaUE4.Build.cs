// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;


public class CarlaUE4 : ModuleRules
{
	private bool IsWindows(ReadOnlyTargetRules Target)
	{
		return (Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32);
	}

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "./GazeEventDetection/")); }
    }

	public CarlaUE4(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivatePCHHeaderFile = "CarlaUE4.h";

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG" });

		if (Target.Type == TargetType.Editor)
		{
			PublicDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
		}
		else
		{
			// only build this carla exception in Editor mode
			PublicDefinitions.Add("NO_DREYEVR_EXCEPTIONS");
		}
		// Add module for SteamVR support with UE4
		PublicDependencyModuleNames.AddRange(new string[] { "HeadMountedDisplay" });

		// SRanipal plugin for Windows
		if (IsWindows(Target))
		{ // SRanipal unfortunately only works on Windows
			bEnableExceptions = true; // enable unwind semantics for C++-style exceptions
			PrivateDependencyModuleNames.AddRange(new string[] { "SRanipalEye", "LogitechWheelPlugin" });
			PrivateIncludePathModuleNames.AddRange(new string[] { "SRanipalEye" });
            // Add OpenCV
            LoadOpenCV(Target);
            bEnableUndefinedIdentifierWarnings = false;

            // Include IBDT headers
            PublicIncludePaths.AddRange(new string[] { Path.Combine(ThirdPartyPath, "IBDT") });
		}


		PrivateDependencyModuleNames.AddRange( new string[] {"ImageWriteQueue"});
		
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}

	public bool LoadOpenCV(ReadOnlyTargetRules Target)
	{
	    bool isLibrarySupported = false;
	    string OpenCVPath = Path.Combine(ThirdPartyPath, "opencv");

	    string LibPath = "";
	    bool isdebug = ((Target.Configuration == UnrealTargetConfiguration.Debug)
	                        && Target.bDebugBuildsActuallyUseDebugCRT);
        if (Target.Platform == UnrealTargetPlatform.Win64){
            LibPath = Path.Combine(OpenCVPath, "libs", "win64");
            isLibrarySupported = true;
        }
        else{
            string Err = string.Format(
            "{0} dedicated server is made to depend on {1}. We want to avoid this, please correct module dependencies.",
             Target.Platform.ToString(), this.ToString()); System.Console.WriteLine(Err);
        }

        if (isLibrarySupported){
            // Add Include path
            PublicIncludePaths.AddRange(new string[] { Path.Combine(OpenCVPath, "include") });

            //Add library path -- ok this is now deprecated, so using full paths
//             PublicLibraryPaths.Add(LibPath);

            //Add Static Libraries
            PublicAdditionalLibraries.Add(
            Path.Combine(LibPath, "opencv_world320.lib")
            );
            //Add Dynamic Libraries
            PublicDelayLoadDLLs.Add(Path.Combine(LibPath, "opencv_world320.dll"));
            PublicDelayLoadDLLs.Add(Path.Combine(LibPath, "opencv_ffmpeg320_64.dll"));
        }

        PublicDefinitions.Add(string.Format("WITH_OPENCV_BINDING={0}", isLibrarySupported ? 1 : 0));
        return isLibrarySupported;
	}
}


