/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomSoundBaseActions.h
 *
 ****************************************************************************/

#pragma once

#include "AssetTools/AtomAssetTypeActions_Base.h"

// Forward Declaration
class UAtomSoundBase;

class FAtomSoundBaseActions
	: public FAtomAssetTypeActions_Base
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InAssetCategory The category to use for new asset editor menu.
	 */
	FAtomSoundBaseActions(EAssetTypeCategories::Type InAssetCategory);

public:

	//~ FAssetTypeActions_Base interface
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual bool CanFilter() override { return false; }
	//virtual bool CanRename(const FAssetData& InAsset, FText* OutErrorMsg) const override;
	//virtual bool CanDuplicate(const FAssetData& InAsset, FText* OutErrorMsg) const override;
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
	virtual void GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section) override;
	virtual bool AssetsActivatedOverride(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType) override;
	virtual TSharedPtr<SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override;

protected:
	/** Plays the specified sound wave */
	void PlaySound(UAtomSoundBase* Sound) const;

	/** Stops any currently playing sounds */
	void StopSound() const;

	/** Return true if the specified sound is playing */
	bool IsSoundPlaying(UAtomSoundBase* Sound) const;

	/** Return true if the specified asset's sound is playing */
	bool IsSoundPlaying(const FAssetData& AssetData) const;

private:

	/** Handler for when PlaySound is selected */
	void ExecutePlaySound(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const;

	/** Handler for when StopSound is selected */
	void ExecuteStopSound(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const;

	/** Returns true if only one sound is selected to play */
	bool CanExecutePlayCommand(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const;

	/** Handler for Mute is selected  */
	void ExecuteMuteSound(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const;

	/** Handler for Solo is selected  */
	void ExecuteSoloSound(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const;

	/** Returns true if the mute state is set.  */
	bool IsActionCheckedMute(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const;

	/** Returns true if the solo state is set.  */
	bool IsActionCheckedSolo(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const;

	/** Returns true if its possible to mute a sound */
	bool CanExecuteMuteCommand(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const;

	/** Returns true if its possible to solo a sound */
	bool CanExecuteSoloCommand(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const;
};
