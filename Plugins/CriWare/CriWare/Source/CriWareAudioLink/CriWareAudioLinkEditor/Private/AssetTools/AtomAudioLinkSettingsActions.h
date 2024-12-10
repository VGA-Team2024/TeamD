/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareAudioLinkEditor
 * File     : AtomAudioLinkSettingsActions.cpp
 *
 ****************************************************************************/

#pragma once

#include "AssetTools/AtomAssetTypeActions_Base.h"
#include "Styling/ISlateStyle.h"

/**
 * Implements an action for UAtomAudioLinkSettings assets.
 */
class FAtomAudioLinkSettingsActions
	: public FAtomAssetTypeActions_Base
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InStyle The style set to use for asset editor toolkits.
	 */
	FAtomAudioLinkSettingsActions(EAssetTypeCategories::Type InAssetCategory);

public:

	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual const TArray<FText>& GetSubMenus() const override;
};
