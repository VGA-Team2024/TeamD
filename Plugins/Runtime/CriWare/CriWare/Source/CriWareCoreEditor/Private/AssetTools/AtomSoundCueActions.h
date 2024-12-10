/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomSoundCueActions.h
 *
 ****************************************************************************/

#pragma once

#include "AtomSoundBaseActions.h"

// Forward Declarations
class UAtomSoundCue;

class FAtomSoundCueActions
	: public FAtomSoundBaseActions
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InAssetCategory The category to use for new asset editor menu.
	 */
	FAtomSoundCueActions(EAssetTypeCategories::Type InAssetCategory);

public:

	//~ FAssetTypeActions_Base interface
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual bool CanFilter() override { return true; }
	virtual bool CanRename(const FAssetData& InAsset, FText* OutErrorMsg) const override { return false; }
	virtual bool CanDuplicate(const FAssetData& InAsset, FText* OutErrorMsg) const override { return false; }
	virtual void GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section) override;
	virtual const TArray<FText>& GetSubMenus() const override;
	virtual TSharedPtr<SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override;
	//virtual bool AssetsActivatedOverride(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType) override;
	//virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

private:

	/** Take selected AtomCues and combine, as much as possible, them to using shared attenuation settings */
	void ExecuteConsolidateAttenuation(TArray<TWeakObjectPtr<UAtomSoundCue>> Objects);

	/** Returns true if more than one cue is selected to consolidate */
	bool CanExecuteConsolidateCommand(TArray<TWeakObjectPtr<UAtomSoundCue>> Objects) const;

	/** */
	EVisibility GetThumbnailLoopOverlayVisibility(const FAssetData AssetData) const;
	EVisibility GetThumbnailLockOverlayVisibility(const FAssetData AssetData) const;
};
