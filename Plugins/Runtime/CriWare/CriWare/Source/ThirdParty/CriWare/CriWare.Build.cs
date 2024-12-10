/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : Build Settings of CriWare Module
 * File     : CriWare.Build.cs
 *
 ****************************************************************************/

using System.IO;
namespace UnrealBuildTool.Rules
{
	public class CriWare : ModuleRules
	{
		public CriWare(ReadOnlyTargetRules Target) : base(Target)
		{
			Type = ModuleType.External;

            string CriWarePath = Path.Combine(ModuleDirectory, "cri/");

			if (Target.Platform == UnrealTargetPlatform.Win64) {
				string LibDir = Path.Combine(CriWarePath, "pc", "libs", "x64");

				AddRangePublicSystemLibraries(
					new string[] {
						"mfplat.lib",
						"mfuuid.lib",
					}
				);

				PublicIncludePaths.Add(Path.Combine(CriWarePath + "pc", "include"));
				PublicAdditionalLibraries.AddRange(
					new string[] {
                        Path.Combine(LibDir, "cri_ware_pcx64_le_import.lib")
                    }
                );

                PublicDelayLoadDLLs.Add("cri_ware_pcx64_le.dll");
                RuntimeDependencies.Add(Path.Combine(CriWarePath, "pc", "libs", "x64", "cri_ware_pcx64_le.dll"));
            } else if (Target.Platform == UnrealTargetPlatform.Mac) {
				string LibDir = Path.Combine(CriWarePath, "macosx", "libs");

                PublicFrameworks.AddRange(new string[] { "CoreAudio", "AudioToolbox", "AudioUnit" });
                PublicIncludePaths.Add(CriWarePath + "macosx/include");

                PublicAdditionalLibraries.AddRange(
					new string[] {
                        LibDir + "/libcri_ware_macosx_le.a"
                    }
                );
            } else if (Target.Platform == UnrealTargetPlatform.IOS) {
                string LibDir = Path.Combine(CriWarePath, "ios", "libs");
				PublicFrameworks.AddRange(new string[] { "CoreAudio", "AudioToolbox" });
                PublicIncludePaths.Add(CriWarePath + "ios/include");
                PublicAdditionalLibraries.AddRange(
                    new string[] {
                        LibDir + "/libcri_ware_ios_le.a"
                    }
                );
            } else if (Target.Platform == UnrealTargetPlatform.Android) {
                string LibDir = Path.Combine(CriWarePath, "android", "libs");
				var TargetArchs = new string[] {
					"armeabi-v7a",
					"arm64-v8a",
					"x86",
					"x86_64"
				};

                PublicIncludePaths.Add(CriWarePath + "android/include");
                foreach (var Arch in TargetArchs)
                {
                    PublicAdditionalLibraries.Add(Path.Combine(LibDir, Arch, "libcri_ware_android_le.a"));
                    //System.Console.WriteLine(Path.Combine(LibDir, Arch, "libcri_ware_android_le.a"));
                }
            }
        }

        public void AddPublicSystemLibraries(string LibName)
        {
            string TheObjectName = "PublicSystemLibraries";
            object TheObject = GetType().GetField(TheObjectName).GetValue(this);
            TheObject.GetType().GetMethod("Add").Invoke(TheObject, new object[] { LibName });
        }

        public void AddRangePublicSystemLibraries(string[] LibNameArray)
        {
            string TheObjectName = "PublicSystemLibraries";
            object TheObject = GetType().GetField(TheObjectName).GetValue(this);
            TheObject.GetType().GetMethod("AddRange").Invoke(TheObject, new object[] { LibNameArray });
        }

        public void AddCriLibrariesToPublicAdditionalLibraries(string LibraryPath, string[] Archs, string[] LibFilenames)
        {
            foreach (var Arch in Archs)
            {
                foreach (var LibFilename in LibFilenames)
                {
                    PublicAdditionalLibraries.AddRange(
                        new string[] {
                            Path.Combine(LibraryPath, Arch, LibFilename)
                        }
                    );
                }
            }
        }
    }
}
