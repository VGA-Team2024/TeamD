/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomWaveBankActions.cpp
 *
 ****************************************************************************/

#include "AtomWaveBankActions.h"

#include "EditorFramework/AssetImportData.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#include "Atom/AtomWaveBank.h"
#include "Atom/AtomSoundSimple.h"

#include "AtomDefaults.h"
#include "AtomEditorStyle.h"
#include "Toolkits/AtomSoundBankEditorToolkit.h"
#include "Factories/AtomSoundSimpleFactory.h"

#define LOCTEXT_NAMESPACE "AtomWaveBankActions"

 /*
  * Atom Wave Bank Actions
  *****************************************************************************/

FAtomWaveBankActions::FAtomWaveBankActions(EAssetTypeCategories::Type InAssetCategory, const TSharedRef<ISlateStyle>& InStyle)
	: FAtomAssetTypeActions_Base(InAssetCategory)
	, Style(InStyle)
{}

bool FAtomWaveBankActions::CanFilter()
{
	return true;
}

FText FAtomWaveBankActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomWaveBank", "Atom Wave Bank");
}

UClass* FAtomWaveBankActions::GetSupportedClass() const
{
	return UAtomWaveBank::StaticClass();
}

FColor FAtomWaveBankActions::GetTypeColor() const
{
	return Atom::DefaultWaveBankColor;
}

bool FAtomWaveBankActions::IsImportedAsset() const
{
	return true;
}

void FAtomWaveBankActions::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (auto& Asset : TypeAssets)
	{
		const auto AtomWaveBank = CastChecked<UAtomWaveBank>(Asset);
		if (AtomWaveBank->AssetImportData)
		{
			AtomWaveBank->AssetImportData->ExtractFilenames(OutSourceFilePaths);
		}
	}
}

void FAtomWaveBankActions::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	auto WaveBanks = GetTypedWeakObjectPtrs<UAtomWaveBank>(InObjects);

	Section.AddMenuEntry(
		"AtomWaveBank_CreateSimpleSound",
		LOCTEXT("AtomWaveBank_CreateSimpleSound", "Create Atom Simple Sound"),
		LOCTEXT("AtomWaveBank_CreateSimpleSound", "Creates a simple sound assets using all sound waves of each selected wave bank."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.SoundSimple"),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAtomWaveBankActions::ExecuteCreateSimpleSound, WaveBanks),
			FCanExecuteAction()
		)
	);
}

void FAtomWaveBankActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (UObject* Obj : InObjects)
	{
		if (UAtomSoundBank* SoundBank = Cast<UAtomSoundBank>(Obj))
		{
			TSharedRef<FAtomSoundBankEditorToolkit> EditorToolkit = MakeShareable(new FAtomSoundBankEditorToolkit(Style));
			EditorToolkit->Initialize(Mode, EditWithinLevelEditor, SoundBank);
		}
	}
}

void FAtomWaveBankActions::ExecuteCreateSimpleSound(TArray<TWeakObjectPtr<UAtomWaveBank>> WaveBanks)
{
	const FString DefaultSuffix = TEXT("_AtomSimpleSound");

	if (WaveBanks.Num() > 0 && WaveBanks[0].Get())
	{
		// Determine an appropriate name
		FString Name;
		FString PackagePath;

		// Create the factory used to generate the asset
		UAtomSoundSimpleFactory* Factory = NewObject<UAtomSoundSimpleFactory>();
		Factory->SoundWaves.Reset();

		FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		AssetToolsModule.Get().CreateUniqueAssetName(WaveBanks[0]->GetOutermost()->GetName(), DefaultSuffix, PackagePath, Name);

		for (auto ObjIt = WaveBanks.CreateConstIterator(); ObjIt; ++ObjIt)
		{
			UAtomWaveBank* WaveBank = (*ObjIt).Get();
			if (WaveBank)
			{
				for (int Index = 0; Index < WaveBank->GetNumWaves(); Index++)
				{
					if (UAtomSoundWave* Wave = WaveBank->GetSoundWave(Index))
					{
						Factory->SoundWaves.Add(Wave);
					}
				}
			}
		}

		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().CreateNewAsset(Name, FPackageName::GetLongPackagePath(PackagePath), UAtomSoundSimple::StaticClass(), Factory);
	}
}

#undef LOCTEXT_NAMESPACE
