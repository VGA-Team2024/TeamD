/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomSoundWaveActions.h
 *
 ****************************************************************************/

#pragma once

#include "AtomSoundBaseActions.h"

#include "Atom/AtomSoundCue.h"

// Forward Declarations
class UAtomSoundWave;

/**
 * Implements an action for UAtomSoundWave assets.
 */
class FAtomSoundWaveActions
	: public FAtomSoundBaseActions
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InStyle The style set to use for asset editor toolkits.
	 */
	FAtomSoundWaveActions(EAssetTypeCategories::Type InAssetCategory);

public:

	//~ FAssetTypeActions_Base interface
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual bool CanFilter() override { return true; }
	virtual bool CanRename(const FAssetData& InAsset, FText* OutErrorMsg) const override { return true; }
	virtual bool CanDuplicate(const FAssetData& InAsset, FText* OutErrorMsg) const override { return false; }
	virtual void GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section) override;
	virtual const TArray<FText>& GetSubMenus() const override;
	virtual TSharedPtr<SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override;

private:

	void ExecuteCreateSimpleSound(TArray<TWeakObjectPtr<UAtomSoundWave>> Waves);

	EVisibility GetThumbnailLoopOverlayVisibility(const FAssetData AssetData) const;
};

class FMySoundActions
	: public FAtomSoundBaseActions
{
public:

	FMySoundActions(EAssetTypeCategories::Type InAssetCategory) : FAtomSoundBaseActions(InAssetCategory) {}


	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_mysound", "MySound"); }
	virtual FColor GetTypeColor() const override { return FColor::Cyan; }
	virtual UClass* GetSupportedClass() const override { return UMySound::StaticClass(); }
	virtual const TArray<FText>& GetSubMenus() const override;
};
