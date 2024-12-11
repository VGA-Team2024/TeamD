/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomSoundCueActions.cpp
 *
 ****************************************************************************/

#include "AtomSoundCueActions.h"

#include "EditorFramework/AssetImportData.h"
#include "Misc/PackageName.h"
#include "ToolMenus.h"
#include "Factories/AtomAttenuationFactory.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Styling/SlateStyleRegistry.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Images/SImage.h"

#include "Atom/AtomSoundCue.h"
#include "Atom/AtomAttenuation.h"

#include "AtomDefaults.h"

#define LOCTEXT_NAMESPACE "AtomSoundCueActions"

FAtomSoundCueActions::FAtomSoundCueActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomSoundBaseActions(InAssetCategory)
{
	// Disable the icon overlay to keep waveform thumbnail clean.
	bEnableIconOverlay = false;
}

FText FAtomSoundCueActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomSoundCue", "Atom Sound Cue");
}

FColor FAtomSoundCueActions::GetTypeColor() const
{
	return Atom::DefaultSoundCueColor;
}

UClass* FAtomSoundCueActions::GetSupportedClass() const
{
	return UAtomSoundCue::StaticClass();
}

void FAtomSoundCueActions::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	auto AtomCues = GetTypedWeakObjectPtrs<UAtomSoundCue>(InObjects);
	FAtomSoundBaseActions::GetActions(InObjects, Section);

	Section.AddMenuEntry(
		"AtomSoundCue_ConsolidateAttenuation",
		LOCTEXT("AtomSoundCue_ConsolidateAttenuation", "Consolidate Attenuation"),
		LOCTEXT("AtomSoundCue_ConsolidateAttenuationTooltip", "Creates shared attenuation packages for sound cues with identical override attenuation settings."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAtomSoundCueActions::ExecuteConsolidateAttenuation, AtomCues),
			FCanExecuteAction::CreateSP(this, &FAtomSoundCueActions::CanExecuteConsolidateCommand, AtomCues)
		)
	);
}

bool FAtomSoundCueActions::CanExecuteConsolidateCommand(TArray<TWeakObjectPtr<UAtomSoundCue>> Objects) const
{
	return Objects.Num() > 1;
}

void FAtomSoundCueActions::ExecuteConsolidateAttenuation(TArray<TWeakObjectPtr<UAtomSoundCue>> Objects)
{
	TMap<FAtomAttenuationSettings*, TArray<UAtomSoundCue*>> UnmatchedAttenuations;

	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UAtomSoundCue* AtomCue = (*ObjIt).Get();
		bool bFound = false;
		if (AtomCue && AtomCue->bOverrideAttenuation)
		{
			for (auto UnmatchedIt = UnmatchedAttenuations.CreateIterator(); UnmatchedIt; ++UnmatchedIt)
			{
				// Found attenuation settings to consolidate together
				if (AtomCue->AttenuationOverrides == *UnmatchedIt.Key())
				{
					UnmatchedIt.Value().Add(AtomCue);
					bFound = true;
					break;
				}
			}
			if (!bFound)
			{
				UnmatchedAttenuations.FindOrAdd(&AtomCue->AttenuationOverrides).Add(AtomCue);
			}
		}
	}

	if (UnmatchedAttenuations.Num() > 0)
	{
		FString DefaultSuffix;
		TArray<UObject*> ObjectsToSync;

		FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
		UAtomAttenuationFactory* Factory = NewObject<UAtomAttenuationFactory>();

		for (auto UnmatchedIt = UnmatchedAttenuations.CreateConstIterator(); UnmatchedIt; ++UnmatchedIt)
		{
			if (UnmatchedIt.Value().Num() > 1)
			{
				FString Name;
				FString PackageName;
				CreateUniqueAssetName("/Game/Sounds/AtomAttenuations/SharedAttenuation", DefaultSuffix, PackageName, Name);

				UAtomAttenuation* AtomAttenuation = Cast<UAtomAttenuation>(AssetToolsModule.Get().CreateAsset(Name, FPackageName::GetLongPackagePath(PackageName), UAtomAttenuation::StaticClass(), Factory));
				if (AtomAttenuation)
				{
					AtomAttenuation->Attenuation = *UnmatchedIt.Key();

					for (int32 AtomCueIndex = 0; AtomCueIndex < UnmatchedIt.Value().Num(); ++AtomCueIndex)
					{
						UAtomSoundCue* AtomCue = UnmatchedIt.Value()[AtomCueIndex];
						AtomCue->bOverrideAttenuation = false;
						AtomCue->AttenuationSettings = AtomAttenuation;
						AtomCue->MarkPackageDirty();
					}
				}
			}
		}

		if (ObjectsToSync.Num() > 0)
		{
			FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
			ContentBrowserModule.Get().SyncBrowserToAssets(ObjectsToSync);
		}
	}
}

/*void FAtomSoundCueActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto SoundCue = Cast<UAtomSoundCue>(*ObjIt);
		if (SoundCue != NULL)
		{
			IAudioEditorModule* AudioEditorModule = &FModuleManager::LoadModuleChecked<IAudioEditorModule>("AudioEditor");
			AudioEditorModule->CreateSoundCueEditor(Mode, EditWithinLevelEditor, SoundCue);
		}
	}
}*/


const TArray<FText>& FAtomSoundCueActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus
	{
		LOCTEXT("AssetAtomSourceSubMenu", "Source")
	};

	return SubMenus;
}

TSharedPtr<SWidget> FAtomSoundCueActions::GetThumbnailOverlay(const FAssetData& AssetData) const
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
				SNew(SWrapBox)
				+ SWrapBox::Slot()
				.HAlign(HAlign)
				.VAlign(VAlign_Top)
				[
					// Looping
					SNew(SBorder)
					.BorderImage(FAppStyle::GetNoBrush())
					.Visibility_Raw(this, &FAtomSoundCueActions::GetThumbnailLoopOverlayVisibility, AssetData)
					.Padding(Padding)
					.HAlign(HAlign)
					.VAlign(VAlign_Top)
					[
						SNew(SImage)	
						.ToolTipText(LOCTEXT("FAtomSoundCueActions_LoopIcon_ToolTip", "This Cue is looping."))
						.Image(LoopIcon)
					]
				]
				+ SWrapBox::Slot()
				.HAlign(HAlign)
				.VAlign(VAlign_Top)
				[
					// Locked
					SNew(SBorder)
					.BorderImage(FAppStyle::GetNoBrush())
					.Visibility_Raw(this, &FAtomSoundCueActions::GetThumbnailLockOverlayVisibility, AssetData)
					.Padding(Padding)
					.HAlign(HAlign)
					.VAlign(VAlign_Top)
					[
						SNew(SImage)
						.ToolTipText(LOCTEXT("FAtomSoundCueActions_LockIcon_ToolTip", "This Cue parameters are locked by Atom Craft. Any parameters or modulations set in Engine for playback are ignoreed."))
						.Image(LockIcon)
					]
				]
			]
			+SOverlay::Slot()
			[
				PreviewOverlay.ToSharedRef()
			];
	}

	return PreviewOverlay;
}

EVisibility FAtomSoundCueActions::GetThumbnailLoopOverlayVisibility(const FAssetData AssetData) const
{
	FAssetTagValueRef AssetTagValue = AssetData.TagsAndValues.FindTag(TEXT("IsOneShot"));
	if (AssetTagValue.IsSet())
	{
		return AssetTagValue.GetValue() == TEXT("false") ? EVisibility::Visible : EVisibility::Collapsed;
	}

	return EVisibility::Hidden;
}

EVisibility FAtomSoundCueActions::GetThumbnailLockOverlayVisibility(const FAssetData AssetData) const
{
	FAssetTagValueRef AssetTagValue = AssetData.TagsAndValues.FindTag(TEXT("IgnorePlayerParameters"));
	if (AssetTagValue.IsSet())
	{
		return AssetTagValue.GetValue() == TEXT("true") ? EVisibility::Visible : EVisibility::Collapsed;
	}

	return EVisibility::Hidden;
}

#undef LOCTEXT_NAMESPACE
