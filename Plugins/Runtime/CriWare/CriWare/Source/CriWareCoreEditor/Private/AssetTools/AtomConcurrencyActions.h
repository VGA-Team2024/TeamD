/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomConcurrencyActions.h
 *
 ****************************************************************************/

#pragma once

#include "AssetTools/AtomAssetTypeActions_Base.h"

class FAtomConcurrencyActions
	: public FAtomAssetTypeActions_Base
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InAssetCategory The category to use for new asset editor menu.
	 */
	FAtomConcurrencyActions(EAssetTypeCategories::Type InAssetCategory);

public:

	//~ IAssetTypeActions interface
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual const TArray<FText>& GetSubMenus() const override;
};
