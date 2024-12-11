/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomModulationActions.h
 *
 ****************************************************************************/

#pragma once

#include "AssetTools/AtomAssetTypeActions_Base.h"

class FAtomModulationControlBusActions
	: public FAtomAssetTypeActions_Base
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InAssetCategory The category to use for new asset editor menu.
	 */
	FAtomModulationControlBusActions(EAssetTypeCategories::Type InAssetCategory);

public:

	//~ FAssetTypeActions_Base interface
	//virtual bool CanFilter() override;
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual bool CanRename(const FAssetData& InAsset, FText* OutErrorMsg) const override;
	virtual bool CanDuplicate(const FAssetData& InAsset, FText* OutErrorMsg) const override;
	virtual void GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section) override;
	virtual const TArray<FText>& GetSubMenus() const override;
	//virtual bool AssetsActivatedOverride(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType) override;
	//virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

private:

	/** Asset category to use for this asset. */
	EAssetTypeCategories::Type AssetCategory;
};

class FAtomModulationControlBusMixActions
	: public FAtomAssetTypeActions_Base
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InAssetCategory The category to use for new asset editor menu.
	 */
	FAtomModulationControlBusMixActions(EAssetTypeCategories::Type InAssetCategory);

public:

	//~ FAssetTypeActions_Base interface
	//virtual bool CanFilter() override;
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual bool CanRename(const FAssetData& InAsset, FText* OutErrorMsg) const override;
	virtual bool CanDuplicate(const FAssetData& InAsset, FText* OutErrorMsg) const override;
	virtual void GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section) override;
	virtual const TArray<FText>& GetSubMenus() const override;
	//virtual bool AssetsActivatedOverride(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType) override;
	//virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

private:

	/** Asset category to use for this asset. */
	EAssetTypeCategories::Type AssetCategory;
};

class FAtomModulationGeneratorActions
	: public FAtomAssetTypeActions_Base
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InAssetCategory The category to use for new asset editor menu.
	 */
	FAtomModulationGeneratorActions(EAssetTypeCategories::Type InAssetCategory);

	//~ FAssetTypeActions_Base Interface
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual const TArray<FText>& GetSubMenus() const override;
	virtual UClass* GetSupportedClass() const override;
};

class FAtomModulationParameterActions
	: public FAtomAssetTypeActions_Base
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InAssetCategory The category to use for new asset editor menu.
	 */
	FAtomModulationParameterActions(EAssetTypeCategories::Type InAssetCategory);

	//~ FAssetTypeActions_Base Interface
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual const TArray<FText>& GetSubMenus() const override;
	virtual UClass* GetSupportedClass() const override;
};
