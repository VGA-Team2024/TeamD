/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomEffectPresetActions.cpp
 *
 ****************************************************************************/

#include "AtomEffectPresetActions.h"

#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#include "HAL/IConsoleManager.h"
#include "Settings/EditorLoadingSavingSettings.h"
#include "Toolkits/IToolkitHost.h"
#include "Toolkits/SimpleAssetEditor.h"

#include "CriWareWidgetSubsystem.h"
#include "Atom/AtomEffectPresetWidgetInterface.h"

#include "Toolkits/AtomEffectPresetEditorToolkit.h"

#define LOCTEXT_NAMESPACE "AtomEffectPresetActions"


namespace AtomEffectPresets
{
	static const TArray<FText> SubMenus
	{
		FText(LOCTEXT("AtomEffectSubMenu", "Effects"))
	};
}

FAtomBusEffectPresetActions::FAtomBusEffectPresetActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomAssetTypeActions_Base(InAssetCategory)
{}

FAtomEffectPresetActions::FAtomEffectPresetActions(UAtomEffectPreset* InEffectPreset, EAssetTypeCategories::Type InAssetCategory)
	: FAtomAssetTypeActions_Base(InAssetCategory)
	, EffectPreset(InEffectPreset)
{
}

FText FAtomEffectPresetActions::GetName() const
{
	FText AssetActionName = EffectPreset->GetAssetActionName();
	if (AssetActionName.IsEmpty())
	{
		FString ClassName;
		EffectPreset->GetClass()->GetName(ClassName);
		ensureMsgf(false, TEXT("U%sGetAssetActionName not implemented. Please check that EFFECT_PRESET_METHODS(EffectClassName) is at the top of the declaration of %s."), *ClassName, *ClassName);
		FString DefaultName = ClassName + FString(TEXT(" (Error: EFFECT_PRESET_METHODS() Not Used in Class Declaration)"));
		return FText::FromString(DefaultName);
	}
	else
	{
		return EffectPreset->GetAssetActionName();
	}
}

UClass* FAtomEffectPresetActions::GetSupportedClass() const
{
	UClass* SupportedClass = EffectPreset->GetSupportedClass();
	if (SupportedClass == nullptr)
	{
		FString ClassName;
		EffectPreset->GetClass()->GetName(ClassName);
		ensureMsgf(false, TEXT("U%s::GetSupportedClass not implemented. Please check that EFFECT_PRESET_METHODS(EffectClassName) is at the top of the declaration of %s."), *ClassName, *ClassName);
		return EffectPreset->GetClass();
	}
	else
	{
		return SupportedClass;
	}
}

const TArray<FText>& FAtomBusEffectPresetActions::GetSubMenus() const
{
	return AtomEffectPresets::SubMenus;
}

const TArray<FText>& FAtomEffectPresetActions::GetSubMenus() const
{
	return AtomEffectPresets::SubMenus;
}

/*const TArray<FText>& FAtomEffectSourcePresetChainActions::GetSubMenus() const
{
	return EffectPresets::SubMenus;
}*/

/*const TArray<FText>& FAtomEffectSourcePresetActions::GetSubMenus() const
{
	return EffectPresets::SubMenus;
}*/

void FAtomEffectPresetActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> ToolkitHost)
{
	EToolkitMode::Type Mode = ToolkitHost.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	TArray<UObject*> Objects = InObjects;
	if (UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr)
	{
		if (UCriWareWidgetSubsystem* WidgetSubsystem = GEngine ? GEngine->GetEngineSubsystem<UCriWareWidgetSubsystem>() : nullptr)
		{
			for (UObject* Object : InObjects)
			{
				UAtomEffectPreset* Preset = Cast<UAtomEffectPreset>(Object);
				if (!Preset)
				{
					continue;
				}

				auto FilterFunction = [InPresetClass = Object->GetClass()](UUserWidget* UserWidget)
				{
					TSubclassOf<UAtomEffectPreset> PresetClass = IAtomEffectPresetWidgetInterface::Execute_GetClass(UserWidget);
					while (PresetClass)
					{
						if (PresetClass == InPresetClass)
						{
							return true;
						}

						PresetClass = PresetClass->GetSuperClass();
					}

					return false;
				};

				TArray<UUserWidget*> UserWidgets = WidgetSubsystem->CreateUserWidgets(*World, UAtomEffectPresetWidgetInterface::StaticClass(), FilterFunction);
				if (!UserWidgets.IsEmpty())
				{
					TSharedRef<FAtomEffectPresetEditorToolkit> PresetEditor = MakeShared<FAtomEffectPresetEditorToolkit>();
					PresetEditor->Init(Mode, ToolkitHost, Preset, UserWidgets);
					Objects.Remove(Object);
				}
			}
		}
	}

	if (!Objects.IsEmpty())
	{
		FAssetTypeActions_Base::OpenAssetEditor(Objects, ToolkitHost);
	}
}

#undef LOCTEXT_NAMESPACE
