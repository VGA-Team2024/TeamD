
#pragma once

#include "AtomSoundscapeAssetTypeActions_Base.h"

class FAtomSoundscapeColorActions
	: public FAtomSoundscapeAssetTypeActions_Base
{
public:

	FAtomSoundscapeColorActions(EAssetTypeCategories::Type InAssetCategory);

	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_SoundscapeColor", "Soundscape Color"); }
	virtual FColor GetTypeColor() const override { return FColor(0, 175, 175); }
	virtual const TArray<FText>& GetSubMenus() const override;
	virtual UClass* GetSupportedClass() const override;
};
