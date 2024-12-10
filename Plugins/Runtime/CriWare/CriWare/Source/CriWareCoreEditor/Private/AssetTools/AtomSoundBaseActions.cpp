/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomSoundBaseActions.cpp
 *
 ****************************************************************************/

#include "AtomSoundBaseActions.h"

#include "ToolMenus.h"
#include "Editor.h"
#include "EditorStyleSet.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"

#include "CriWareEditor.h"
#include "Atom/AtomRuntimeManager.h"
#include "Atom/AtomDebug.h"
#include "Atom/AtomComponent.h"
#include "Atom/AtomSoundBase.h"
#include "Atom/AtomSoundCue.h"
#include "Atom/AtomSoundWave.h"

#include "AtomDefaults.h"

#define LOCTEXT_NAMESPACE "AtomSoundBaseActions"

FAtomSoundBaseActions::FAtomSoundBaseActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomAssetTypeActions_Base(InAssetCategory)
{}

FText FAtomSoundBaseActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomSoundBase", "Atom Sound Base");
}

FColor FAtomSoundBaseActions::GetTypeColor() const
{ 
	return Atom::DefaultSoundBaseColor; 
}

UClass* FAtomSoundBaseActions::GetSupportedClass() const
{
	return UAtomSoundBase::StaticClass();
}

void FAtomSoundBaseActions::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	auto Sounds = GetTypedWeakObjectPtrs<UAtomSoundBase>(InObjects);

	Section.AddMenuEntry(
		"AtomSound_PlaySound",
		LOCTEXT("AtomSound_PlaySound", "Play"),
		LOCTEXT("AtomSound_PlaySoundTooltip", "Plays the selected sound."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "MediaAsset.AssetActions.Play.Small"),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAtomSoundBaseActions::ExecutePlaySound, Sounds),
			FCanExecuteAction::CreateSP(this, &FAtomSoundBaseActions::CanExecutePlayCommand, Sounds)
		)
	);

	Section.AddMenuEntry(
		"AtomSound_StopSound",
		LOCTEXT("AtomSound_StopSound", "Stop"),
		LOCTEXT("AtomSound_StopSoundTooltip", "Stops the selected sounds."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "MediaAsset.AssetActions.Stop.Small"),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAtomSoundBaseActions::ExecuteStopSound, Sounds),
			FCanExecuteAction()
		)
	);

	Section.AddMenuEntry(
		"AtomSound_SoundMute",
		LOCTEXT("AtomSound_MuteSound", "Mute"),
		LOCTEXT("AtomSound_MuteSoundTooltip", "Mutes the selected sounds."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "MediaAsset.AssetActions.Mute.Small"),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAtomSoundBaseActions::ExecuteMuteSound, Sounds),
			FCanExecuteAction::CreateSP(this, &FAtomSoundBaseActions::CanExecuteMuteCommand, Sounds),
			FIsActionChecked::CreateSP(this, &FAtomSoundBaseActions::IsActionCheckedMute, Sounds)
		),
		EUserInterfaceActionType::ToggleButton
	);

	Section.AddMenuEntry(
		"AtomSound_StopSolo",
		LOCTEXT("AtomSound_SoloSound", "Solo"),
		LOCTEXT("AtomSound_SoloSoundTooltip", "Solos the selected sounds."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "MediaAsset.AssetActions.Solo.Small"),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAtomSoundBaseActions::ExecuteSoloSound, Sounds),
			FCanExecuteAction::CreateSP(this, &FAtomSoundBaseActions::CanExecuteSoloCommand, Sounds),
			FIsActionChecked::CreateSP(this, &FAtomSoundBaseActions::IsActionCheckedSolo, Sounds)
		),
		EUserInterfaceActionType::ToggleButton
	);

	Section.AddMenuEntry("Atom_GenerateThumbnail", 
		LOCTEXT("GenerateThumbnail", "Generate Thumbnail"), 
		LOCTEXT("GenerateThumbnail_Tooltip", "Generate a thumbnail for this asset."), 
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "AssetEditor.SaveThumbnail"), 
		FUIAction(
			FExecuteAction::CreateLambda([](TArray<TWeakObjectPtr<UAtomSoundBase>> Objects)
			{
				for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
				{
					UAtomSoundBase* Sound = (*ObjIt).Get();
					if (Sound)
					{
						Sound->GenerateWaveSnapshot();
						Sound->MarkPackageDirty();
					}
				}
			}, Sounds)
		)
	);
}

bool FAtomSoundBaseActions::AssetsActivatedOverride(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType)
{
	if (ActivationType == EAssetTypeActivationMethod::Previewed)
	{
		UAtomSoundBase* TargetSound = nullptr;

		for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
		{
			TargetSound = Cast<UAtomSoundBase>(*ObjIt);
			if (TargetSound)
			{
				// Only target the first valid sound cue
				break;
			}
		}

		UAtomComponent* PreviewComp = GCriWareEditor->GetPreviewAtomComponent();
		if (PreviewComp && PreviewComp->IsPlaying())
		{
			// Already previewing a sound, if it is the target cue then stop it, otherwise play the new one
			if (!TargetSound || PreviewComp->Sound == TargetSound)
			{
				StopSound();
			}
			else
			{
				PlaySound(TargetSound);
			}
		}
		else
		{
			// Not already playing, play the target sound cue if it exists
			PlaySound(TargetSound);
		}
		return true;
	}
	return false;
}


void FAtomSoundBaseActions::PlaySound(UAtomSoundBase* Sound) const
{
	if (Sound)
	{
		GCriWareEditor->PlayPreviewSound(Sound);
	}
	else
	{
		StopSound();
	}
}

void FAtomSoundBaseActions::StopSound() const
{
	GCriWareEditor->ResetPreviewAtomComponent();
}

bool FAtomSoundBaseActions::IsSoundPlaying(UAtomSoundBase* Sound) const
{
	UAtomComponent* PreviewComp = GCriWareEditor->GetPreviewAtomComponent();
	return PreviewComp && PreviewComp->Sound == Sound && PreviewComp->IsPlaying();
}

bool FAtomSoundBaseActions::IsSoundPlaying(const FAssetData& AssetData) const
{
	const UAtomComponent* PreviewComp = GCriWareEditor->GetPreviewAtomComponent();
	if (PreviewComp && PreviewComp->Sound && PreviewComp->IsPlaying())
	{
		if (PreviewComp->Sound->GetFName() == AssetData.AssetName)
		{
			if (PreviewComp->Sound->GetOutermost()->GetFName() == AssetData.PackageName)
			{
				return true;
			}
		}
	}

	return false;
}

void FAtomSoundBaseActions::ExecutePlaySound(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const
{
	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UAtomSoundBase* Sound = (*ObjIt).Get();
		if (Sound)
		{
			// Only play the first valid sound
			PlaySound(Sound);
			break;
		}
	}
}

void FAtomSoundBaseActions::ExecuteStopSound(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const
{
	StopSound();
}

TSharedPtr<SWidget> FAtomSoundBaseActions::GetThumbnailOverlay(const FAssetData& AssetData) const
{
	auto OnGetDisplayBrushLambda = [this, AssetData]() -> const FSlateBrush*
	{
		if (IsSoundPlaying(AssetData))
		{
			return FAppStyle::GetBrush("MediaAsset.AssetActions.Stop.Large");
		}

		return FAppStyle::GetBrush("MediaAsset.AssetActions.Play.Large");
	};

	auto OnClickedLambda = [this, AssetData]() -> FReply
	{
		if (IsSoundPlaying(AssetData))
		{
			StopSound();
		}
		else
		{
			// Load and play sound
			PlaySound(Cast<UAtomSoundBase>(AssetData.GetAsset()));
		}
		return FReply::Handled();
	};

	auto OnToolTipTextLambda = [this, AssetData]() -> FText
	{
		if (IsSoundPlaying(AssetData))
		{
			return LOCTEXT("AtomThumbnail_StopSoundToolTip", "Stop selected sound");
		}

		return LOCTEXT("AtomThumbnail_PlaySoundToolTip", "Play selected sound");
	};

	TSharedPtr<SBox> Box;
	SAssignNew(Box, SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(FMargin(2));

	auto OnGetVisibilityLambda = [this, Box, AssetData]() -> EVisibility
	{
		if (Box.IsValid() && (Box->IsHovered() || IsSoundPlaying(AssetData)))
		{
			return EVisibility::Visible;
		}

		return EVisibility::Hidden;
	};

	TSharedPtr<SButton> Widget;
	SAssignNew(Widget, SButton)
		.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
		.ToolTipText_Lambda(OnToolTipTextLambda)
		.Cursor(EMouseCursor::Default) // The outer widget can specify a DragHand cursor, so we need to override that here
		.ForegroundColor(FSlateColor::UseForeground())
		.IsFocusable(false)
		.OnClicked_Lambda(OnClickedLambda)
		.Visibility_Lambda(OnGetVisibilityLambda)
		[
			SNew(SImage)
			.Image_Lambda(OnGetDisplayBrushLambda)
		];

	Box->SetContent(Widget.ToSharedRef());
	Box->SetVisibility(EVisibility::Visible);

	// draw the base thumbnail overlay under this overlay if enabled 
	if (bEnableIconOverlay)
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				FAtomAssetTypeActions_Base::GetThumbnailOverlay(AssetData).ToSharedRef()
			]
			+ SOverlay::Slot()
			[
				Box.ToSharedRef()
			];
	}

	return Box;
}

bool FAtomSoundBaseActions::CanExecutePlayCommand(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const
{
	return Objects.Num() == 1;
}

void FAtomSoundBaseActions::ExecuteMuteSound(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const
{
#if ENABLE_ATOM_DEBUG
	if (FAtomRuntimeManager* RuntimeManager = GCriWareEditor->GetAtomRuntimeManager())
	{
		Atom::FAtomDebugger& Debugger = RuntimeManager->GetDebugger();

		// In a selection that consists of some already muted, toggle everything in the same direction,
		// to avoid AB problem.

		bool bAnyMuted = IsActionCheckedMute(Objects);
		for (TWeakObjectPtr<UAtomSoundBase> SoundBase : Objects)
		{
			if (UAtomSoundCue* Cue = Cast<UAtomSoundCue>(SoundBase.Get()))
			{
				Debugger.SetMuteSoundCue(FName(Cue->GetSoundName()), !bAnyMuted);
			}
			else if (UAtomSoundWave* Wave = Cast<UAtomSoundWave>(SoundBase.Get()))
			{
				Debugger.SetMuteSoundWave(FName(Wave->GetSoundName()), !bAnyMuted);
			}
		}
	}
#endif
}

void FAtomSoundBaseActions::ExecuteSoloSound(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const
{
#if ENABLE_ATOM_DEBUG
	if (FAtomRuntimeManager* RuntimeManager = GCriWareEditor->GetAtomRuntimeManager())
	{
		Atom::FAtomDebugger& Debugger = RuntimeManager->GetDebugger();

		// In a selection that consists of some already soloed, toggle everything in the same direction,
		// to avoid AB problem.

		bool bAnySoloed = IsActionCheckedSolo(Objects);
		for (TWeakObjectPtr<UAtomSoundBase> SoundBase : Objects)
		{
			if (UAtomSoundCue* Cue = Cast<UAtomSoundCue>(SoundBase.Get()))
			{
				Debugger.SetSoloSoundCue(FName(Cue->GetSoundName()), !bAnySoloed);
			}
			else if (UAtomSoundWave* Wave = Cast<UAtomSoundWave>(SoundBase.Get()))
			{
				Debugger.SetSoloSoundWave(FName(Wave->GetSoundName()), !bAnySoloed);
			}
		}
	}
#endif
}

bool FAtomSoundBaseActions::IsActionCheckedMute(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const
{
#if ENABLE_ATOM_DEBUG
	if (FAtomRuntimeManager* RuntimeManager = GCriWareEditor->GetAtomRuntimeManager())
	{
		// If *any* of the selection are muted, show the tick box as ticked.
		Atom::FAtomDebugger& Debugger = RuntimeManager->GetDebugger();
		for (TWeakObjectPtr<UAtomSoundBase> SoundBase : Objects)
		{
			if (UAtomSoundCue* Cue = Cast<UAtomSoundCue>(SoundBase.Get()))
			{
				if (Debugger.IsMuteSoundCue(FName(Cue->GetSoundName())))
				{
					return true;
				}
			}
			else if (UAtomSoundWave* Wave = Cast<UAtomSoundWave>(SoundBase.Get()))
			{
				if (Debugger.IsMuteSoundWave(FName(Wave->GetSoundName())))
				{
					return true;
				}
			}
		}
	}
#endif
	return false;
}

bool FAtomSoundBaseActions::IsActionCheckedSolo(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const
{
#if ENABLE_ATOM_DEBUG
	if (FAtomRuntimeManager* RuntimeManager = GCriWareEditor->GetAtomRuntimeManager())
	{
		// If *any* of the selection are solod, show the tick box as ticked.
		Atom::FAtomDebugger& Debugger = RuntimeManager->GetDebugger();
		for (TWeakObjectPtr<UAtomSoundBase> SoundBase : Objects)
		{
			if (UAtomSoundCue* Cue = Cast<UAtomSoundCue>(SoundBase.Get()))
			{
				if (Debugger.IsSoloSoundCue(FName(Cue->GetSoundName())))
				{
					return true;
				}
			}
			else if (UAtomSoundWave* Wave = Cast<UAtomSoundWave>(SoundBase.Get()))
			{
				if (Debugger.IsSoloSoundWave(FName(Wave->GetSoundName())))
				{
					return true;
				}
			}
		}
	}
#endif
	return false;
}

bool FAtomSoundBaseActions::CanExecuteMuteCommand(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const
{
#if ENABLE_ATOM_DEBUG
	if (FAtomRuntimeManager* RuntimeManager = GCriWareEditor->GetAtomRuntimeManager())
	{
		// Allow muting if we're not Soloing.
		Atom::FAtomDebugger& Debugger = RuntimeManager->GetDebugger();
		for (TWeakObjectPtr<UAtomSoundBase> SoundBase : Objects)
		{
			if (UAtomSoundCue* Cue = Cast<UAtomSoundCue>(SoundBase.Get()))
			{
				if (Debugger.IsSoloSoundCue(FName(Cue->GetSoundName())))
				{
					return false;
				}
			}
			else if (UAtomSoundWave* Wave = Cast<UAtomSoundWave>(SoundBase.Get()))
			{
				if (Debugger.IsSoloSoundWave(FName(Wave->GetSoundName())))
				{
					return false;
				}
			}
		}

		// Ok.
		return true;
	}
#endif
	return false;
}

bool FAtomSoundBaseActions::CanExecuteSoloCommand(TArray<TWeakObjectPtr<UAtomSoundBase>> Objects) const
{
#if ENABLE_ATOM_DEBUG
	if (FAtomRuntimeManager* RuntimeManager = GCriWareEditor->GetAtomRuntimeManager())
	{
		// Allow Soloing if we're not Muting.
		Atom::FAtomDebugger& Debugger = RuntimeManager->GetDebugger();
		for (TWeakObjectPtr<UAtomSoundBase> SoundBase : Objects)
		{
			if (UAtomSoundCue* Cue = Cast<UAtomSoundCue>(SoundBase.Get()))
			{
				if (Debugger.IsMuteSoundCue(FName(Cue->GetSoundName())))
				{
					return false;
				}
			}
			else if (UAtomSoundWave* Wave = Cast<UAtomSoundWave>(SoundBase.Get()))
			{
				if (Debugger.IsMuteSoundWave(FName(Wave->GetSoundName())))
				{
					return false;
				}
			}
		}

		// Ok.
		return true;
	}
#endif
	return false;
}

#undef LOCTEXT_NAMESPACE
