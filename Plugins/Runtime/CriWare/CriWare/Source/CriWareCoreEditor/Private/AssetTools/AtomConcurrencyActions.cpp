/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomConcurrencyActions.cpp
 *
 ****************************************************************************/

#include "AtomConcurrencyActions.h"

#include "EditorFramework/AssetImportData.h"
#include "Misc/PackageName.h"
#include "ToolMenus.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#include "Atom/AtomConcurrency.h"

#include "CriWareCoreEditorAtomSettings.h"

#include "AtomDefaults.h"

#define LOCTEXT_NAMESPACE "AtomConcurrencyActions"

/* FAtomConcurrencyActions
 *****************************************************************************/

FAtomConcurrencyActions::FAtomConcurrencyActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomAssetTypeActions_Base(InAssetCategory)
{}

FText FAtomConcurrencyActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomConcurrency", "Atom Concurrency");
}

UClass* FAtomConcurrencyActions::GetSupportedClass() const
{
	return UAtomConcurrency::StaticClass();
}

FColor FAtomConcurrencyActions::GetTypeColor() const
{
	return Atom::DefaultConcurrencyColor;
}

const TArray<FText>& FAtomConcurrencyActions::GetSubMenus() const
{
	if (GetDefault<UCriWareCoreEditorAtomSettings>()->bPinAtomSoundConcurrencyInAssetMenu)
	{
		static const TArray<FText> AssetTypeActionSubMenu;
		return AssetTypeActionSubMenu;
	}

	static const TArray<FText> AssetTypeActionSubMenu
	{
		LOCTEXT("AssetAtomConcurrencySubMenu", "Mix"),
	};
	return AssetTypeActionSubMenu;
}

#undef LOCTEXT_NAMESPACE
