/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2024 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Build Settings of CriWareSoundXR Module
 * File     : CriWareSoundXR.Build.cs
 *
 ****************************************************************************/

using UnrealBuildTool;

public class CriWareSoundXR : ModuleRules
{
	public CriWareSoundXR(ReadOnlyTargetRules Target) : base(Target)
    {

		PrivateIncludePaths.AddRange(
			new string[] {
                "CriWareSoundXR/Private",
            }
		);

	    PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"CriWareCore",
				"CriWare",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
            }
		);
	}
}
