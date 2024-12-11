/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomAttenuationActions.cpp
 *
 ****************************************************************************/

#include "AtomAttenuationActions.h"

#include "Atom/AtomAttenuation.h"

#include "CriWareCoreEditorAtomSettings.h"

#include "AtomDefaults.h"

#define LOCTEXT_NAMESPACE "AtomAttenuationActions"

FAtomAttenuationActions::FAtomAttenuationActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomAssetTypeActions_Base(InAssetCategory)
{}

FText FAtomAttenuationActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomAttenuation", "Atom Attenuation");
}

UClass* FAtomAttenuationActions::GetSupportedClass() const
{
	return UAtomAttenuation::StaticClass();
}

FColor FAtomAttenuationActions::GetTypeColor() const
{
	return Atom::DefaultAttenuationColor;
}

const TArray<FText>& FAtomAttenuationActions::GetSubMenus() const
{
	if (GetDefault<UCriWareCoreEditorAtomSettings>()->bPinAtomSoundAttenuationInAssetMenu)
	{
		static const TArray<FText> AssetTypeActionSubMenu;
		return AssetTypeActionSubMenu;
	}
	
	static const TArray<FText> AssetTypeActionSubMenu
	{
		LOCTEXT("AssetSoundAttenuationSubMenu", "Spatialization"),
	};
	
	return AssetTypeActionSubMenu;
}

#undef LOCTEXT_NAMESPACE
