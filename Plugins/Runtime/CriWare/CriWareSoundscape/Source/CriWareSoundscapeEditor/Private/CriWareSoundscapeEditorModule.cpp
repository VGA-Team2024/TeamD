
#include "CriWareSoundScapeEditorModule.h"

#include "CoreMinimal.h"

#include "AssetTools/AtomSoundscapeColorActions.h"
#include "AssetTools/AtomSoundscapePaletteActions.h"

IMPLEMENT_MODULE(FCriWareSoundscapeEditorModule, CriWareSoundscapeEditor)

void FCriWareSoundscapeEditorModule::StartupModule()
{
	// Register the editor plugin asset type actions.
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Find or register ADX advanced category in context menu.
	auto AdxAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("CriWare ADX")), FText::FromString(TEXT("ADX Atom")));

	// Asset Type Actions
	auto RegisterTypeActions = [this, InAssetTools = &AssetTools](const TSharedRef<IAssetTypeActions> Actions)
	{
		InAssetTools->RegisterAssetTypeActions(Actions);
		RegisteredAssetTypeActions.Add(Actions);
	};

	RegisterTypeActions(MakeShared<FAtomSoundscapeColorActions>(AdxAssetCategoryBit));
	RegisterTypeActions(MakeShared<FAtomSoundscapePaletteActions>(AdxAssetCategoryBit));
}

void FCriWareSoundscapeEditorModule::ShutdownModule()
{
	// Unregister Asset Type Actions
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

		for (auto Action : RegisteredAssetTypeActions)
		{
			AssetToolsModule.UnregisterAssetTypeActions(Action);
		}
	}
}
