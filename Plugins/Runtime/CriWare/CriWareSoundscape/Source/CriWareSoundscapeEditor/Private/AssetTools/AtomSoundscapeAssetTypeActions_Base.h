/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareSoundscape
 * File     : AtomSoundscapeAssetTypeActions_Base.h
 *
 ****************************************************************************/

#pragma once

#include "AssetTypeActions_Base.h"

class FAtomSoundscapeAssetTypeActions_Base
	: public FAssetTypeActions_Base
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InAssetCategory The category to use for new asset editor menu.
	 */
	FAtomSoundscapeAssetTypeActions_Base(EAssetTypeCategories::Type InAssetCategory)
		: bEnableIconOverlay(true)
		, AssetCategory(InAssetCategory)
	{}

	//~ FAssetTypeActions_Base interface
	virtual uint32 GetCategories() override { return AssetCategory; }
	virtual TSharedPtr<SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override;

	/** Enable the thumbnail overlay. */
	bool bEnableIconOverlay;

private:

	/** Asset category to use for this asset. */
	EAssetTypeCategories::Type AssetCategory;
};
