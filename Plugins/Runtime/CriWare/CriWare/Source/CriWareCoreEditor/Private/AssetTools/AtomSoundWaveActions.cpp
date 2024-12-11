/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomSoundWaveActions.cpp
 *
 ****************************************************************************/

#include "AtomSoundWaveActions.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Styling/SlateStyleRegistry.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Images/SImage.h"

#include "Atom/AtomSoundWave.h"
#include "Atom/AtomSoundSimple.h"

#include "AtomDefaults.h"
#include "Factories/AtomSoundSimpleFactory.h"

#define LOCTEXT_NAMESPACE "AtomSoundWaveActions"

/*
 * Atom Sound Wave Actions
 *****************************************************************************/

FAtomSoundWaveActions::FAtomSoundWaveActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomSoundBaseActions(InAssetCategory)
{
	// Disable the icon overlay to keep waveform thumbnail clean.
	bEnableIconOverlay = false;
}

FText FAtomSoundWaveActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomSoundWave", "Atom Sound Wave");
}

FColor FAtomSoundWaveActions::GetTypeColor() const
{
	return Atom::DefaultSoundWaveColor;
}

UClass* FAtomSoundWaveActions::GetSupportedClass() const
{
	return UAtomSoundWave::StaticClass();
}

void FAtomSoundWaveActions::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	FAtomSoundBaseActions::GetActions(InObjects, Section);

	auto AtomWaves = GetTypedWeakObjectPtrs<UAtomSoundWave>(InObjects);
	FAtomSoundBaseActions::GetActions(InObjects, Section);

	Section.AddMenuEntry(
		"AtomWaveBank_CreateSimpleSound",
		LOCTEXT("AtomWaveBank_CreateSimpleSound", "Create Atom Simple Sound"),
		LOCTEXT("AtomWaveBank_CreateSimpleSound", "Creates a simple sound assets using all sound waves of each selected wave bank."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.SoundSimple"),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAtomSoundWaveActions::ExecuteCreateSimpleSound, AtomWaves),
			FCanExecuteAction()
		)
	);

	/*Section.AddMenuEntry(
		"AtomSoundCue_ConsolidateAttenuation",
		LOCTEXT("AtomSoundCue_ConsolidateAttenuation", "Consolidate Attenuation"),
		LOCTEXT("AtomSoundCue_ConsolidateAttenuationTooltip", "Creates shared attenuation packages for sound cues with identical override attenuation settings."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAtomSoundCueActions::ExecuteConsolidateAttenuation, AtomCues),
			FCanExecuteAction::CreateSP(this, &FAtomSoundCueActions::CanExecuteConsolidateCommand, AtomCues)
		)
	);*/
}

void FAtomSoundWaveActions::ExecuteCreateSimpleSound(TArray<TWeakObjectPtr<UAtomSoundWave>> Waves)
{
	const FString DefaultSuffix = TEXT("_AtomSimpleSound");

	if (Waves.Num() > 0 && Waves[0].Get())
	{
		// Determine an appropriate name
		FString Name;
		FString PackagePath;

		FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		AssetToolsModule.Get().CreateUniqueAssetName(Waves[0]->GetOutermost()->GetName(), DefaultSuffix, PackagePath, Name);

		// Create the factory used to generate the asset
		UAtomSoundSimpleFactory* Factory = NewObject<UAtomSoundSimpleFactory>();
		Factory->SoundWaves.Reset();
		for (auto ObjIt = Waves.CreateConstIterator(); ObjIt; ++ObjIt)
		{
			if (UAtomSoundWave* Wave = (*ObjIt).Get())
			{
				Factory->SoundWaves.Add(Wave);
			}
		}

		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().CreateNewAsset(Name, FPackageName::GetLongPackagePath(PackagePath), UAtomSoundSimple::StaticClass(), Factory);
	}
}

const TArray<FText>& FAtomSoundWaveActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus
	{
		LOCTEXT("AssetAtomSoundWaveSubMenu", "Source")
	};

	return SubMenus;
}

TSharedPtr<SWidget> FAtomSoundWaveActions::GetThumbnailOverlay(const FAssetData& AssetData) const
{
	TSharedPtr<SWidget> PreviewOverlay = FAtomSoundBaseActions::GetThumbnailOverlay(AssetData);

	const ISlateStyle* AtomEditorStyle = FSlateStyleRegistry::FindSlateStyle("AtomEditorStyle");
	if (ensure(AtomEditorStyle))
	{
		// From UE 5.3 we can use the right side before the place was used by Source Control's status overlay.
		const EHorizontalAlignment HAlign = (FEngineVersion::Current().GetMajor() == 5 && FEngineVersion::Current().GetMinor() >= 3) ? HAlign_Right : HAlign_Left;
		const FMargin Padding = HAlign == HAlign_Right ? FMargin(0.0f, 3.0f, 3.0f, 0.0f) : FMargin(3.0f, 3.0f, 0.0f, 0.0f);
		const FSlateBrush* LockIcon = AtomEditorStyle->GetBrush("CriWare.LockIcon");
		const FSlateBrush* LoopIcon = AtomEditorStyle->GetBrush("CriWare.LoopIcon");

		return SNew(SOverlay)
			+SOverlay::Slot()
			.HAlign(HAlign)
			.VAlign(VAlign_Top)
			[
				//SNew(SWrapBox)
				//+ SWrapBox::Slot()
				//.HAlign(HAlign)
				//.VAlign(VAlign_Top)
				//[
					// Looping
					SNew(SBorder)
					.BorderImage(FAppStyle::GetNoBrush())
					.Visibility_Raw(this, &FAtomSoundWaveActions::GetThumbnailLoopOverlayVisibility, AssetData)
					.Padding(Padding)
					.HAlign(HAlign)
					.VAlign(VAlign_Top)
					[
						SNew(SImage)	
						.ToolTipText(LOCTEXT("FAtomSoundWaveActions_LoopIcon_ToolTip", "This Wave is looping."))
						.Image(LoopIcon)
					]
				//]
			]
			+SOverlay::Slot()
			[
				PreviewOverlay.ToSharedRef()
			];
	}

	return PreviewOverlay;
}

EVisibility FAtomSoundWaveActions::GetThumbnailLoopOverlayVisibility(const FAssetData AssetData) const
{
	FAssetTagValueRef AssetTagValue = AssetData.TagsAndValues.FindTag(TEXT("IsOneShot"));
	if (AssetTagValue.IsSet())
	{
		return AssetTagValue.GetValue() == TEXT("false") ? EVisibility::Visible : EVisibility::Collapsed;
	}

	return EVisibility::Hidden;
}


const TArray<FText>& FMySoundActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus
	{
		LOCTEXT("AssetAtomSoundWaveSubMenu", "Source")
	};

	return SubMenus;
}

#undef LOCTEXT_NAMESPACE
