/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomSoundSimpleActions.cpp
 *
 ****************************************************************************/

#include "AtomSoundSimpleActions.h"

#include "AtomDefaults.h"
#include "Atom/AtomSoundSimple.h"

#include "CriWareCoreEditorAtomSettings.h"

#define LOCTEXT_NAMESPACE "AtomSoundSimpleActions"

 /*
  * Atom Sound Simple Actions
  *****************************************************************************/

FAtomSoundSimpleActions::FAtomSoundSimpleActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomSoundBaseActions(InAssetCategory)
{}

uint32 FAtomSoundSimpleActions::GetCategories()
{
	return FAtomSoundBaseActions::GetCategories();
}

FText FAtomSoundSimpleActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomSoundSimple", "Atom Sound Simple");
}

FColor FAtomSoundSimpleActions::GetTypeColor() const
{
	return Atom::DefaultSoundSimpleColor;
}

UClass* FAtomSoundSimpleActions::GetSupportedClass() const
{
	return UAtomSoundSimple::StaticClass();
}

const TArray<FText>& FAtomSoundSimpleActions::GetSubMenus() const
{
	if (GetDefault<UCriWareCoreEditorAtomSettings>()->bPinAtomSoundSimpleInAssetMenu)
	{
		static const TArray<FText> AssetTypeActionSubMenu;
		return AssetTypeActionSubMenu;
	}
	static const TArray<FText> SubMenus
	{
		LOCTEXT("AssetAtomSourceSubMenu", "Source")
	};

	return SubMenus;
}

#undef LOCTEXT_NAMESPACE
