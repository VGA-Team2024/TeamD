
#include "AtomSoundscapeColorActions.h"

#include "AtomSoundscapeColor.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FAtomSoundscapeColorActions::FAtomSoundscapeColorActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomSoundscapeAssetTypeActions_Base(InAssetCategory)
{
}

UClass* FAtomSoundscapeColorActions::GetSupportedClass() const
{
	return UAtomSoundscapeColor::StaticClass();
}

const TArray<FText>& FAtomSoundscapeColorActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus
	{
		LOCTEXT("AssetAtomSoundscapeSubMenu", "Soundscape")
	};
	return SubMenus;
}

#undef LOCTEXT_NAMESPACE
