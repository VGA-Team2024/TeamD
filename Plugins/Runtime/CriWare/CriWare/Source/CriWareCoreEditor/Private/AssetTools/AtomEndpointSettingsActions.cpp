/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomEndpointSettingsActions.cpp
 *
 ****************************************************************************/

#include "AtomEndpointSettingsActions.h"

#include "Extensions/IAtomEndpoint.h"

#include "AtomDefaults.h"

#define LOCTEXT_NAMESPACE "AtomEndpointSettingsActions"

 /* FAtomEndpointSettingsActions
  *****************************************************************************/

FAtomEndpointSettingsActions::FAtomEndpointSettingsActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomAssetTypeActions_Base(InAssetCategory)
{

}

FText FAtomEndpointSettingsActions::GetName() const
{
   return LOCTEXT("AssetTypeActions_AtomEndpointSettings", "Atom Endpoint Settings");
}

UClass* FAtomEndpointSettingsActions::GetSupportedClass() const
{
	return UAtomEndpointSettingsBase::StaticClass();
}

FColor FAtomEndpointSettingsActions::GetTypeColor() const
{
	return FColor(100, 100, 100);
}

const TArray<FText>& FAtomEndpointSettingsActions::GetSubMenus() const
{
    static const TArray<FText> SubMenus
    {
        LOCTEXT("AssetAtomSoundRendererSubMenu", "Sound Renderer")
    };

    return SubMenus;
}

#undef LOCTEXT_NAMESPACE
