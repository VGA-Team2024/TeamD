/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomRackActions.cpp
 *
 ****************************************************************************/

#include "AtomRackActions.h"

#include "EditorFramework/AssetImportData.h"

#include "Atom/AtomRack.h"
#include "Atom/AtomBus.h"

#include "AtomDefaults.h"
#include "Toolkits/AtomRackEditorToolkit.h"

#define LOCTEXT_NAMESPACE "AtomRackActions"

/*
 * Atom Rack Actions
 *****************************************************************************/

FAtomRackActions::FAtomRackActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomAssetTypeActions_Base(InAssetCategory)
{}

FText FAtomRackActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomRack", "Atom Rack");
}

UClass* FAtomRackActions::GetSupportedClass() const
{
	return UAtomRack::StaticClass();
}

FColor FAtomRackActions::GetTypeColor() const 
{ 
	return Atom::DefaultRackColor;
}

void FAtomRackActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (UObject* Obj : InObjects)
	{
		if (UAtomRackBase* AtomRack = Cast<UAtomRackBase>(Obj))
		{
			TSharedRef<FAtomRackEditorToolkit> EditorToolkit = MakeShareable(new FAtomRackEditorToolkit());
			EditorToolkit->Initialize(AtomRack, Mode, EditWithinLevelEditor);

			//IAudioEditorModule* AudioEditorModule = &FModuleManager::LoadModuleChecked<IAudioEditorModule>("AudioEditor");
			//AudioEditorModule->CreateSoundSubmixEditor(Mode, EditWithinLevelEditor, AtomRack);
		}
	}
}

bool FAtomRackActions::AssetsActivatedOverride(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType)
{
	TSet<UAtomRackBase*> RacksToSelect;
	IAssetEditorInstance* Editor = nullptr;
	for (UObject* Obj : InObjects)
	{
		if (UAtomRackBase* RackToSelect = Cast<UAtomRackBase>(Obj))
		{
			if (!Editor)
			{
				Editor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Obj, true);
			}
			RacksToSelect.Add(RackToSelect);
		}
	}

	if (Editor)
	{
		static_cast<FAtomRackEditorToolkit*>(Editor)->SelectRacks(RacksToSelect);
		return true;
	}

	return false;
}

const TArray<FText>& FAtomRackActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus
	{
		LOCTEXT("AssetAtomSoundRendererSubMenu", "Sound Renderer")
	};

	return SubMenus;
}

/*
 * Atom Soundfield Rack Actions
 *****************************************************************************/

FAtomSoundfieldRackActions::FAtomSoundfieldRackActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomRackActions(InAssetCategory)
{}

FText FAtomSoundfieldRackActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomSoundfieldRack", "Atom Soundfield Rack");
}

UClass* FAtomSoundfieldRackActions::GetSupportedClass() const
{
	return UAtomSoundfieldRack::StaticClass();
}

FColor FAtomSoundfieldRackActions::GetTypeColor() const
{
	return Atom::SoundfieldDefaultRackColor;
}

/*
 * Atom Native Rack Actions
 *****************************************************************************/

FAtomEndpointRackActions::FAtomEndpointRackActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomRackActions(InAssetCategory)
{}

FText FAtomEndpointRackActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomEndpointRack", "Atom Endpoint Rack");
}

UClass* FAtomEndpointRackActions::GetSupportedClass() const
{
	return UAtomEndpointRack::StaticClass();
}

FColor FAtomEndpointRackActions::GetTypeColor() const
{
	return Atom::EndpointDefaultRackColor;
}

/*
 * Atom Native Rack Actions
 *****************************************************************************/

FAtomSoundfieldEndpointRackActions::FAtomSoundfieldEndpointRackActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomRackActions(InAssetCategory)
{}

FText FAtomSoundfieldEndpointRackActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomSoundfieldEndpointRack", "Atom Soundfield Endpoint Rack");
}

UClass* FAtomSoundfieldEndpointRackActions::GetSupportedClass() const
{
	return UAtomSoundfieldEndpointRack::StaticClass();
}

FColor FAtomSoundfieldEndpointRackActions::GetTypeColor() const
{
	return Atom::SoundfieldEndpointDefaultRackColor;
}

/*
 * Atom Bus Actions
 *****************************************************************************/

FAtomBusActions::FAtomBusActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomAssetTypeActions_Base(InAssetCategory)
{}

FText FAtomBusActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomBus", "Atom Bus");
}

UClass* FAtomBusActions::GetSupportedClass() const
{
	return UAtomBus::StaticClass();
}

FColor FAtomBusActions::GetTypeColor() const
{
	return Atom::DefaultBusColor;
}

#undef LOCTEXT_NAMESPACE
