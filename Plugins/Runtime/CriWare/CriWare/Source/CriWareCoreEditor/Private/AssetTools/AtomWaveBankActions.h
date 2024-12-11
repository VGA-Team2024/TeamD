/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomWaveBankActions.h
 *
 ****************************************************************************/

#pragma once

#include "Styling/ISlateStyle.h"

#include "AssetTools/AtomAssetTypeActions_Base.h"

class UAtomWaveBank;

/**
 * Implements an action for UAtomWaveBank assets.
 */
class FAtomWaveBankActions
	: public FAtomAssetTypeActions_Base
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InStyle The style set to use for asset editor toolkits.
	 */
	FAtomWaveBankActions(EAssetTypeCategories::Type InAssetCategory, const TSharedRef<ISlateStyle>& InStyle);

public:

	//~ FAssetTypeActions_Base interface
	virtual bool CanFilter() override;
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual bool IsImportedAsset() const override;
	virtual void GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const override;
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
	virtual void GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section) override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;

private:

	void ExecuteCreateSimpleSound(TArray<TWeakObjectPtr<UAtomWaveBank>> WaveBanks);

	TSharedRef<ISlateStyle> Style;
};
