
#pragma once

#include "AtomSoundscapeAssetTypeActions_Base.h"

class FAtomSoundscapePaletteActions
	: public FAtomSoundscapeAssetTypeActions_Base
{
public:

	FAtomSoundscapePaletteActions(EAssetTypeCategories::Type InAssetCategory);

	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_SoundscapePalette", "Soundscape Palette"); }
	virtual FColor GetTypeColor() const override { return FColor(0, 125, 255); }
	virtual const TArray<FText>& GetSubMenus() const override;
	virtual UClass* GetSupportedClass() const override;
};
