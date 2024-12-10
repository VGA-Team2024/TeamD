/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : Build Settings of CriWareCore Module
 * File     : CriWareCore.Build.cs
 *
 ****************************************************************************/
using System.IO;

namespace UnrealBuildTool.Rules
{
    public class CriWareCore : ModuleRules
    {
        public CriWareCore(ReadOnlyTargetRules Target) : base(Target)
        {
            PrivateIncludePaths.AddRange(new string[]
                {
                    "CriWareCore/Private/Atom",
                    "CriWareCore/Private",
                }
            );

            PrivateDependencyModuleNames.AddRange(
                new string[] {
                    "CoreUObject",
                    "RHI",
                    "RenderCore",
                    "SlateCore",
                    "UMG",
                    "DeveloperSettings",
                    "HeadMountedDisplay",
                    "Projects",
					"SignalProcessing",
					"AudioPlatformConfiguration",
					"GameplayTags",
					"Networking",
				}
            );

            PublicDependencyModuleNames.AddRange(
                new string[] {
                    "Engine",
                    "Core",
                    "CriWare",
				}
            );

            /* backward compatibility with 4.21 */
            BuildVersion Version;
            if (BuildVersion.TryRead(BuildVersion.GetDefaultFileName(), out Version) &&
                Version.MajorVersion == 4 && Version.MinorVersion < 22)
            {
                PublicDependencyModuleNames.Add("ShaderCore");
            }

            string engine_path = System.IO.Path.GetFullPath(Target.RelativeEnginePath);

            PrivateIncludePaths.AddRange(
                new string[] {
                    engine_path + "Source/Runtime/Engine/Private",
                }
            );

            UnrealTargetPlatform TargetPlatformValue;

            string CriPlatformLabel = "NOT_SUPPORTED";
            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                CriPlatformLabel = "pc";
            }
            else if (Target.Platform == UnrealTargetPlatform.Mac)
            {
                CriPlatformLabel = "macosx";
            }
            else if (Target.Platform == UnrealTargetPlatform.IOS)
            {
                CriPlatformLabel = "ios";
            }
            else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Android))
            {
                CriPlatformLabel = "android";
            }
            else if (UnrealTargetPlatform.TryParse("WinGDK", out TargetPlatformValue) && Target.Platform == TargetPlatformValue)
            {
                CriPlatformLabel = "pc";
            }

            /* CRI のサポート対象であるかどうかを示すをプロパティを追加。他のモジュールスクリプトから参照する */
            AddPublicDefinition(string.Format("WITH_CRIWARE={0}", (CriPlatformLabel == "NOT_SUPPORTED") ? 0 : 1));

            if (CriPlatformLabel == "pc")
            {
                AddPublicSystemLibraries("propsys.lib");
                if (Target.WindowsPlatform.bNeedsLegacyStdioDefinitionsLib)
                {
                    AddPublicSystemLibraries("legacy_stdio_definitions.lib");
                }
            }

            /* CRI プラグイン拡張各種が有効かどうかを調べる。有効ならプロパティを追加 */
            {
                string CriNativeDir = "/../ThirdParty/CriWare/cri/" + CriPlatformLabel;
                /* McDSP */
                {
                    string LibHeaderPath = CriNativeDir + "/include/" + "criafx_mcdsp.h";
                    if (File.Exists(ModuleDirectory + LibHeaderPath))
                    {
                        AddPublicDefinition("CRIWARE_USE_MCDSP=1");
                    }
                }
				/* Sound xR */
				{
					string LibHeaderPath = CriNativeDir + "/include/" + "crisoundxr_le_plugin.h";
					if (File.Exists(ModuleDirectory + LibHeaderPath))
					{
						AddPublicDefinition("CRIWARE_USE_SOUNDXR=1");
					}
				}
                /* LE */
                {
                    string LibHeaderPath = CriNativeDir + "/include/" + "cri_le_xpt.h";
                    if (File.Exists(ModuleDirectory + LibHeaderPath))
                    {
                        AddPublicDefinition("CRIWARE_UE_LE=1");
                    }
                }
            }

            if (Target.bBuildEditor == true)
            {
                PrivateIncludePathModuleNames.AddRange(
                    new string[] {
                        "CriWareCoreEditor"
                    }
                );

                PublicIncludePathModuleNames.AddRange(
                    new string[] {
                        "TargetPlatform",
                    }
                );

                PrivateDependencyModuleNames.AddRange(
                    new string[] {
                        "CriWareCoreEditorSettings"
                    }
                );

                PublicDependencyModuleNames.AddRange(
                    new string[] {
                        "UnrealEd",
                        "Slate",
                        //"Persona"
                    }
                );
            }

			CriWareAudioLink.Apply(this, Target);
		}

        public void AddPublicDefinition(string Definition)
        {
            string PublicDefinitionsPropertyName = "PublicDefinitions";
            // <PublicDefinitions>.Add(Definition)
            object PublicDefinitions = GetType().GetField(PublicDefinitionsPropertyName).GetValue(this);
            PublicDefinitions.GetType().GetMethod("Add").Invoke(PublicDefinitions, new object[] { Definition });
        }

        public void AddPublicSystemLibraries(string LibName)
        {
            string TheObjectName = "PublicSystemLibraries";
            object TheObject = GetType().GetField(TheObjectName).GetValue(this);
            TheObject.GetType().GetMethod("Add").Invoke(TheObject, new object[] { LibName });
        }

		public void AddModule(string Module, string ModuleRootDir = "", bool AddPublic = true)
		{
			string ModuleDir = Path.Combine(ModuleRootDir, Module);

			ConditionalAddModuleDirectory(EpicGames.Core.DirectoryReference.Combine(new EpicGames.Core.DirectoryReference(ModuleDirectory), "..", ModuleDir));

			ExternalDependencies.Add(Path.Combine(ModuleDirectory, "..", ModuleDir, Module + ".Build.cs"));
			if (AddPublic)
			{
				PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "..", ModuleDir, "Public"));
			}
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "..", ModuleDir, "Private"));
		}
	}
}
