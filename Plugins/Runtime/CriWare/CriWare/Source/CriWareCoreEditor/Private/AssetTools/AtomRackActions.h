/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomRackActions.h
 *
 ****************************************************************************/

#pragma once

#include "AssetTools/AtomAssetTypeActions_Base.h"

class FAtomRackActions
	: public FAtomAssetTypeActions_Base
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InAssetCategory The category to use for new asset editor menu.
	 */
	FAtomRackActions(EAssetTypeCategories::Type InAssetCategory);

public:

	//~ FAssetTypeActions_Base Interface
	//virtual bool CanFilter() override;
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;

	virtual const TArray<FText>& GetSubMenus() const override;
	virtual bool AssetsActivatedOverride(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType) override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
};

class FAtomSoundfieldRackActions
	: public FAtomRackActions
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InAssetCategory The category to use for new asset editor menu.
	 */
	FAtomSoundfieldRackActions(EAssetTypeCategories::Type InAssetCategory);

	//~ FAssetTypeActions_Base Interface
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
};

class FAtomEndpointRackActions
	: public FAtomRackActions
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InAssetCategory The category to use for new asset editor menu.
	 */
	FAtomEndpointRackActions(EAssetTypeCategories::Type InAssetCategory);
	
	//~ FAssetTypeActions_Base Interface
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
};

class FAtomSoundfieldEndpointRackActions
	: public FAtomRackActions
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InAssetCategory The category to use for new asset editor menu.
	 */
	FAtomSoundfieldEndpointRackActions(EAssetTypeCategories::Type InAssetCategory);

	//~ FAssetTypeActions_Base Interface
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
};

class FAtomBusActions
	: public FAtomAssetTypeActions_Base
{
public:

	FAtomBusActions(EAssetTypeCategories::Type InAssetCategory);

	//~ FAssetTypeActions_Base Interface
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
};
