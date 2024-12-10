/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomModulationActions.cpp
 *
 ****************************************************************************/

#include "AtomModulationActions.h"

#include "EditorFramework/AssetImportData.h"
#include "Misc/PackageName.h"
#include "ToolMenus.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#include "Atom/Modulation/AtomModulationControlBus.h"
#include "Atom/Modulation/AtomModulationControlBusMix.h"
#include "Atom/Modulation/AtomModulationGenerator.h"

#include "AtomDefaults.h"

#define LOCTEXT_NAMESPACE "AtomModulationActions"

/* FAtomModulationControlBusActions
 *****************************************************************************/

FAtomModulationControlBusActions::FAtomModulationControlBusActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomAssetTypeActions_Base(InAssetCategory)
{}

FText FAtomModulationControlBusActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomModulationControlBus", "Atom Control Bus");
}

UClass* FAtomModulationControlBusActions::GetSupportedClass() const
{
	return UAtomModulationControlBus::StaticClass();
}

FColor FAtomModulationControlBusActions::GetTypeColor() const
{
	return Atom::DefaultModulationControlBusColor;
}

bool FAtomModulationControlBusActions::CanRename(const FAssetData& InAsset, FText* OutErrorMsg) const
{
	return true;
}

bool FAtomModulationControlBusActions::CanDuplicate(const FAssetData& InAsset, FText* OutErrorMsg) const
{
	return true;
}

void FAtomModulationControlBusActions::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	auto AtomControls = GetTypedWeakObjectPtrs<UAtomModulationControlBus>(InObjects);
	FAssetTypeActions_Base::GetActions(InObjects, Section);
}

/*void FAtomModulationControlBusActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto SoundCue = Cast<UAtomSoundCue>(*ObjIt);
		if (SoundCue != NULL)
		{
			IAudioEditorModule* AudioEditorModule = &FModuleManager::LoadModuleChecked<IAudioEditorModule>("AudioEditor");
			AudioEditorModule->CreateSoundCueEditor(Mode, EditWithinLevelEditor, SoundCue);
		}
	}
}*/


const TArray<FText>& FAtomModulationControlBusActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus
	{
		LOCTEXT("AssetAtomModulationControlBusSubMenu", "Modulations")
	};

	return SubMenus;
}

/* FAtomModulationControlBusMixActions
 *****************************************************************************/

FAtomModulationControlBusMixActions::FAtomModulationControlBusMixActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomAssetTypeActions_Base(InAssetCategory)
{}

FText FAtomModulationControlBusMixActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomModulationControlBusMix", "Atom Control Bus Mix");
}

UClass* FAtomModulationControlBusMixActions::GetSupportedClass() const
{
	return UAtomModulationControlBusMix::StaticClass();
}

FColor FAtomModulationControlBusMixActions::GetTypeColor() const
{
	return Atom::DefaultModulationControlBusMixColor;
}

bool FAtomModulationControlBusMixActions::CanRename(const FAssetData& InAsset, FText* OutErrorMsg) const
{
	return true;
}

bool FAtomModulationControlBusMixActions::CanDuplicate(const FAssetData& InAsset, FText* OutErrorMsg) const
{
	return true;
}

void FAtomModulationControlBusMixActions::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	auto AtomControlMixes = GetTypedWeakObjectPtrs<UAtomModulationControlBusMix>(InObjects);
	FAssetTypeActions_Base::GetActions(InObjects, Section);
}

/*void FAtomModulationControlBusMixActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto SoundCue = Cast<UAtomSoundCue>(*ObjIt);
		if (SoundCue != NULL)
		{
			IAudioEditorModule* AudioEditorModule = &FModuleManager::LoadModuleChecked<IAudioEditorModule>("AudioEditor");
			AudioEditorModule->CreateSoundCueEditor(Mode, EditWithinLevelEditor, SoundCue);
		}
	}
}*/


const TArray<FText>& FAtomModulationControlBusMixActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus
	{
		LOCTEXT("AssetAtomModulationControlMixSubMenu", "Modulations")
	};

	return SubMenus;
}

/* FAtomModulationGeneratorActions
 *****************************************************************************/

FAtomModulationGeneratorActions::FAtomModulationGeneratorActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomAssetTypeActions_Base(InAssetCategory)
{}

FText FAtomModulationGeneratorActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomModulationGenerator", "Atom Modulation Generator");
}

UClass* FAtomModulationGeneratorActions::GetSupportedClass() const
{
	return UAtomModulationGenerator::StaticClass();
}

FColor FAtomModulationGeneratorActions::GetTypeColor() const
{
	return Atom::DefaultModulationGeneratorColor;
}

const TArray<FText>& FAtomModulationGeneratorActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus
	{
		LOCTEXT("AssetAtomModulationGeneratorSubMenu", "Modulations")
	};

	return SubMenus;
}

/* FAtomModulationParameterActions
 *****************************************************************************/

FAtomModulationParameterActions::FAtomModulationParameterActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomAssetTypeActions_Base(InAssetCategory)
{}

FText FAtomModulationParameterActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomModulationParameter", "Atom Modulation Parameter");
}

UClass* FAtomModulationParameterActions::GetSupportedClass() const
{
	return UAtomModulationParameter::StaticClass();
}

FColor FAtomModulationParameterActions::GetTypeColor() const
{
	return Atom::DefaultModulationParameterColor;
}

const TArray<FText>& FAtomModulationParameterActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus
	{
		LOCTEXT("AssetAtomModulationParameterSubMenu", "Modulations")
	};

	return SubMenus;
}

#undef LOCTEXT_NAMESPACE
