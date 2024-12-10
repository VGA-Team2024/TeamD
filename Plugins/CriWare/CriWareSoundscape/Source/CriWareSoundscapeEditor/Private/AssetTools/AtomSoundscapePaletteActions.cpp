
#include "AtomSoundscapePaletteActions.h"

#include "AtomSoundscapePalette.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FAtomSoundscapePaletteActions::FAtomSoundscapePaletteActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomSoundscapeAssetTypeActions_Base(InAssetCategory)
{
}

UClass* FAtomSoundscapePaletteActions::GetSupportedClass() const
{
	return UAtomSoundscapePalette::StaticClass();
}

const TArray<FText>& FAtomSoundscapePaletteActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus
	{
		LOCTEXT("AssetAtomSoundscapeSubMenu", "Soundscape")
	};
	return SubMenus;
}

#undef LOCTEXT_NAMESPACE
