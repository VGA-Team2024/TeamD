/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomEffectPresetActions.h
 *
 ****************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundEffectPreset.h"
#include "Widgets/SWidget.h"
#include "Developer/AssetTools/Public/AssetTypeCategories.h"
#include "Developer/AssetTools/Public/IAssetTypeActions.h"
#include "UObject/StrongObjectPtr.h"

#include "Atom/AtomBusEffectPreset.h"

#include "AssetTools/AtomAssetTypeActions_Base.h"
#include "AtomDefaults.h"

 // Forward Declarations
class IToolkitHost;


class FAtomBusEffectPresetActions : public FAtomAssetTypeActions_Base
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InAssetCategory The category to use for new asset editor menu.
	 */
	FAtomBusEffectPresetActions(EAssetTypeCategories::Type InAssetCategory);

public:
	
	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomBusEffectPreset", "Bus Effect Preset"); }
	virtual FColor GetTypeColor() const override { return Atom::DefaultEffectPresetColor; }
	virtual const TArray<FText>& GetSubMenus() const override;
	virtual UClass* GetSupportedClass() const override { return UAtomBusEffectPreset::StaticClass(); }
};

/*class FSoundEffectSourcePresetActions : public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomSourcePreset", "Atom Effect Preset"); }
	virtual FColor GetTypeColor() const override { return FColor(72, 185, 187); }
	virtual const TArray<FText>& GetSubMenus() const override;
	virtual UClass* GetSupportedClass() const override { return USoundEffectSourcePreset::StaticClass(); }
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Sounds; }
};*/

/*class FAtomEffectSourcePresetChainActions : public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomSourcePresetChain", "Atom Effect Preset Chain"); }
	virtual FColor GetTypeColor() const override { return FColor(51, 107, 142); }
	virtual const TArray<FText>& GetSubMenus() const override;
	virtual UClass* GetSupportedClass() const override { return USoundEffectSourcePresetChain::StaticClass(); }
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Sounds; }
};*/

class FAtomEffectPresetActions : public FAtomAssetTypeActions_Base
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InAssetCategory The category to use for new asset editor menu.
	 */
	FAtomEffectPresetActions(UAtomEffectPreset* InEffectPreset, EAssetTypeCategories::Type InAssetCategory);

public:

	//~ Begin FAssetTypeActions_Base
	virtual bool CanFilter() override { return EffectPreset->CanFilter(); }
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override { return EffectPreset->GetPresetColor(); }
	virtual const TArray<FText>& GetSubMenus() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> ToolkitHost) override;
	//~ End FAssetTypeActions_Base

private:

	TStrongObjectPtr<UAtomEffectPreset> EffectPreset;
};
