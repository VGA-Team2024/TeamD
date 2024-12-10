/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareAudioLinkEditor
 * File     : AtomAudioLinkSettingsActions.cpp
 *
 ****************************************************************************/

#include "AtomAudioLinkSettingsActions.h"

#include "AtomAudioLinkSettings.h"

#define LOCTEXT_NAMESPACE "AtomAudioLinkActions"

 /*
  * Atom AudioLink Settings Actions
  *****************************************************************************/

FAtomAudioLinkSettingsActions::FAtomAudioLinkSettingsActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomAssetTypeActions_Base(InAssetCategory)
{}

FText FAtomAudioLinkSettingsActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomAudioLinkSettings", "Atom AudioLink Settings");
}

UClass* FAtomAudioLinkSettingsActions::GetSupportedClass() const
{
	return UAtomAudioLinkSettings::StaticClass();
}

FColor FAtomAudioLinkSettingsActions::GetTypeColor() const
{
	return FColor::Silver;
}

const TArray<FText>& FAtomAudioLinkSettingsActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus
	{
		LOCTEXT("AssetAtomAudioLinkSubMenu", "AudioLink")
	};

	return SubMenus;
}

#undef LOCTEXT_NAMESPACE
