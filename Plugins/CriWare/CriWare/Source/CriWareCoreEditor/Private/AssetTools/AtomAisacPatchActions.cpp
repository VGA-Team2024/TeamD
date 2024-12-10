/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomAisacPatchActions.cpp
 *
 ****************************************************************************/

#include "AtomAisacPatchActions.h"

#include "EditorFramework/AssetImportData.h"
#include "Misc/PackageName.h"
#include "ToolMenus.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#include "Atom/AtomAisacPatch.h"

#include "AtomDefaults.h"

#define LOCTEXT_NAMESPACE "AtomAisacPatchActions"

/* FAtomAisacPatchActions
 *****************************************************************************/

FAtomAisacPatchActions::FAtomAisacPatchActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomAssetTypeActions_Base(InAssetCategory)
{}

FText FAtomAisacPatchActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomAisacPatch", "Atom AISAC Patch");
}

UClass* FAtomAisacPatchActions::GetSupportedClass() const
{
	return UAtomAisacPatch::StaticClass();
}

FColor FAtomAisacPatchActions::GetTypeColor() const
{
	return Atom::DefaultAisacPatchColor;
}

bool FAtomAisacPatchActions::CanRename(const FAssetData& InAsset, FText* OutErrorMsg) const
{
	return false;
}

bool FAtomAisacPatchActions::CanDuplicate(const FAssetData& InAsset, FText* OutErrorMsg) const
{
	return false;
}

void FAtomAisacPatchActions::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	auto AtomAisacPatches = GetTypedWeakObjectPtrs<UAtomAisacPatch>(InObjects);
	FAssetTypeActions_Base::GetActions(InObjects, Section);
}

/*void FAtomAisacPatchActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
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

#undef LOCTEXT_NAMESPACE
