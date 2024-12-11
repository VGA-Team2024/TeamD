/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomSoundClassActions.h
 *
 ****************************************************************************/

#pragma once

#include "AssetTools/AtomAssetTypeActions_Base.h"

// Forward Declarations
class UAtomSoundClass;

class FAtomSoundClassActions
	: public FAtomAssetTypeActions_Base
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InAssetCategory The category to use for new asset editor menu.
	 */
	FAtomSoundClassActions(EAssetTypeCategories::Type InAssetCategory);

public:

	//~ FAssetTypeActions_Base interface
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual bool CanRename(const FAssetData& InAsset, FText* OutErrorMsg) const override;
	virtual bool CanDuplicate(const FAssetData& InAsset, FText* OutErrorMsg) const override;
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;
	virtual const TArray<FText>& GetSubMenus() const override;
	//virtual bool AssetsActivatedOverride(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType) override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

private:

	/** Handler for Mute is selected  */
	void ExecuteMute(TArray<TWeakObjectPtr<UAtomSoundClass>> Objects) const;

	/** Handler for Solo is selected  */
	void ExecuteSolo(TArray<TWeakObjectPtr<UAtomSoundClass>> Objects) const;

	/** Returns true if the mute state is set.  */
	bool IsActionCheckedMute(TArray<TWeakObjectPtr<UAtomSoundClass>> Objects) const;

	/** Returns true if the solo state is set.  */
	bool IsActionCheckedSolo(TArray<TWeakObjectPtr<UAtomSoundClass>> Objects) const;

	/** Returns true if its possible to mute a sound */
	bool CanExecuteMuteCommand(TArray<TWeakObjectPtr<UAtomSoundClass>> Objects) const;

	/** Returns true if its possible to solo a sound */
	bool CanExecuteSoloCommand(TArray<TWeakObjectPtr<UAtomSoundClass>> Objects) const;
};
