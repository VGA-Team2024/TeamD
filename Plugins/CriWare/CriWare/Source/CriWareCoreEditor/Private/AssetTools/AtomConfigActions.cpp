/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomConfigActions.cpp
 *
 ****************************************************************************/

#include "AtomConfigActions.h"

#include "EditorFramework/AssetImportData.h"

#include "Atom/AtomConfig.h"

#include "AtomDefaults.h"

#define LOCTEXT_NAMESPACE "AtomConfigActions"

 /*
  * Atom Config Actions
  *****************************************************************************/

FAtomConfigActions::FAtomConfigActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomAssetTypeActions_Base(InAssetCategory)
{}

bool FAtomConfigActions::CanFilter()
{
	return true;
}

FText FAtomConfigActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomConfig", "Atom Config");
}

UClass* FAtomConfigActions::GetSupportedClass() const
{
	return UAtomConfig::StaticClass();
}

FColor FAtomConfigActions::GetTypeColor() const
{
	return Atom::DefaultConfigColor;
}

bool FAtomConfigActions::IsImportedAsset() const
{
	return true;
}

void FAtomConfigActions::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (auto& Asset : TypeAssets)
	{
		const auto AtomConfig = CastChecked<UAtomConfig>(Asset);
		if (AtomConfig->AssetImportData)
		{
			AtomConfig->AssetImportData->ExtractFilenames(OutSourceFilePaths);
		}
	}
}

/*
 * Atom Dsp Bus settings Actions
 *****************************************************************************/

FAtomDspBusSettingActions::FAtomDspBusSettingActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomAssetTypeActions_Base(InAssetCategory)
{}

bool FAtomDspBusSettingActions::CanFilter()
{
	return true;
}

FText FAtomDspBusSettingActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomDspBusSetting", "Atom Dsp Bus Setting");
}

UClass* FAtomDspBusSettingActions::GetSupportedClass() const
{
	return UAtomDspBusSetting::StaticClass();
}

FColor FAtomDspBusSettingActions::GetTypeColor() const
{
	return Atom::DefaultDspBusSettingColor;
}

bool FAtomDspBusSettingActions::IsImportedAsset() const
{
	return false;
}

bool FAtomDspBusSettingActions::CanRename(const FAssetData& InAsset, FText* OutErrorMsg) const
{
	return false;
}

bool FAtomDspBusSettingActions::CanDuplicate(const FAssetData& InAsset, FText* OutErrorMsg) const
{
	return false;
}


#undef LOCTEXT_NAMESPACE
