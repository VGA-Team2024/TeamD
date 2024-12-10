/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomCueSheetActions.cpp
 *
 ****************************************************************************/

#include "AtomCueSheetActions.h"

#include "EditorFramework/AssetImportData.h"

#include "Atom/AtomCueSheet.h"

#include "AtomDefaults.h"
#include "Toolkits/AtomSoundBankEditorToolkit.h"

#define LOCTEXT_NAMESPACE "AtomCueSheetActions"

FAtomCueSheetActions::FAtomCueSheetActions(EAssetTypeCategories::Type InAssetCategory, const TSharedRef<ISlateStyle>& InStyle)
	: FAtomAssetTypeActions_Base(InAssetCategory)
	, Style(InStyle)
{}

bool FAtomCueSheetActions::CanFilter()
{
	return true;
}

FText FAtomCueSheetActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomCueSheet", "Atom Cue Sheet");
}

UClass* FAtomCueSheetActions::GetSupportedClass() const
{
	return UAtomCueSheet::StaticClass();
}

FColor FAtomCueSheetActions::GetTypeColor() const
{
	return Atom::DefaultCueSheetColor;
}

bool FAtomCueSheetActions::IsImportedAsset() const
{
	return true;
}

void FAtomCueSheetActions::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (auto& Asset : TypeAssets)
	{
		const auto AtomCueSheet = CastChecked<UAtomCueSheet>(Asset);
		if (AtomCueSheet->AssetImportData)
		{
			AtomCueSheet->AssetImportData->ExtractFilenames(OutSourceFilePaths);
		}
	}
}

void FAtomCueSheetActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (UObject* Obj : InObjects)
	{
		if (UAtomSoundBank* SoundBank = Cast<UAtomSoundBank>(Obj))
		{
			TSharedRef<FAtomSoundBankEditorToolkit> EditorToolkit = MakeShareable(new FAtomSoundBankEditorToolkit(Style));
			EditorToolkit->Initialize(Mode, EditWithinLevelEditor, SoundBank);
		}
	}
}

#undef LOCTEXT_NAMESPACE
