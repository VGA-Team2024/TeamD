/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomSoundClassActions.cpp
 *
 ****************************************************************************/

#include "AtomSoundClassActions.h"

#include "EditorFramework/AssetImportData.h"
#include "Misc/PackageName.h"
#include "ToolMenus.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#include "CriWare.h"
#include "CriWareDefines.h"
#include "Atom/AtomSoundClass.h"
#include "Atom/AtomDebug.h"

#include "CriWareCoreEditorAtomSettings.h"

#include "AtomDefaults.h"
#include "Toolkits/AtomSoundClassEditorToolkit.h"

#define LOCTEXT_NAMESPACE "AtomSoundClassActions"

FAtomSoundClassActions::FAtomSoundClassActions(EAssetTypeCategories::Type InAssetCategory)
	: FAtomAssetTypeActions_Base(InAssetCategory)
{}

FText FAtomSoundClassActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AtomSoundClass", "Atom Sound Class");
}

UClass* FAtomSoundClassActions::GetSupportedClass() const
{
	return UAtomSoundClass::StaticClass();
}

FColor FAtomSoundClassActions::GetTypeColor() const
{
	return Atom::DefaultSoundClassColor;
}

bool FAtomSoundClassActions::CanRename(const FAssetData& InAsset, FText* OutErrorMsg) const
{
	return true;
}

bool FAtomSoundClassActions::CanDuplicate(const FAssetData& InAsset, FText* OutErrorMsg) const
{
	return true;
}

void FAtomSoundClassActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	auto Classes = GetTypedWeakObjectPtrs<UAtomSoundClass>(InObjects);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("Atom_MuteSoundClass", "Mute"),
		LOCTEXT("Atom_MuteSoundClassSoundTooltip", "Mutes anything using this AtomClass."),
		FSlateIcon(FAppStyle::Get().GetStyleSetName(), "MediaAsset.AssetActions.Mute.Small"),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAtomSoundClassActions::ExecuteMute, Classes),
			FCanExecuteAction::CreateSP(this, &FAtomSoundClassActions::CanExecuteMuteCommand, Classes),
			FIsActionChecked::CreateSP(this, &FAtomSoundClassActions::IsActionCheckedMute, Classes)
		),
		NAME_None,
		EUserInterfaceActionType::ToggleButton
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("Atom_SoloSoundClass", "Solo"),
		LOCTEXT("Atom_SoloSoundClassTooltip", "Mutes anything except sounds using this AtomClass."),
		FSlateIcon(FAppStyle::Get().GetStyleSetName(), "MediaAsset.AssetActions.Solo.Small"),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAtomSoundClassActions::ExecuteSolo, Classes),
			FCanExecuteAction::CreateSP(this, &FAtomSoundClassActions::CanExecuteSoloCommand, Classes),
			FIsActionChecked::CreateSP(this, &FAtomSoundClassActions::IsActionCheckedSolo, Classes)
		),
		NAME_None,
		EUserInterfaceActionType::ToggleButton
	);
}

void FAtomSoundClassActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UAtomSoundClass* SoundClass = Cast<UAtomSoundClass>(*ObjIt);
		if (SoundClass)
		{
			TSharedRef<FAtomSoundClassEditorToolkit> EditorToolkit = MakeShareable(new FAtomSoundClassEditorToolkit());
			EditorToolkit->Initialize(SoundClass, Mode, EditWithinLevelEditor);

			//IAudioEditorModule* AudioEditorModule = &FModuleManager::LoadModuleChecked<IAudioEditorModule>("AudioEditor");
			//AudioEditorModule->CreateSoundClassEditor(Mode, EditWithinLevelEditor, SoundClass);
		}
	}
}

const TArray<FText>& FAtomSoundClassActions::GetSubMenus() const
{
	if (GetDefault<UCriWareCoreEditorAtomSettings>()->bPinAtomSoundClassInAssetMenu)
	{
		static const TArray<FText> AssetTypeActionSubMenu;
		return AssetTypeActionSubMenu;
	}

	static const TArray<FText> SubMenus
	{
		LOCTEXT("AssetAtomSoundClassMenu", "Classes")
	};

	return SubMenus;
}

void FAtomSoundClassActions::ExecuteMute(TArray<TWeakObjectPtr<UAtomSoundClass>> Objects) const
{
#if ENABLE_ATOM_DEBUG
	if (FAtomRuntimeManager* RuntimeManager = GCriWare->GetAtomRuntimeManager())
	{
		for (TWeakObjectPtr<UAtomSoundClass> ClassPtr : Objects)
		{
			if (UAtomSoundClass* Class = Cast<UAtomSoundClass>(ClassPtr.Get()))
			{
				RuntimeManager->GetDebugger().ToggleMuteSoundClass(Class->GetFName());
			}
		}
	}
#endif
}

void FAtomSoundClassActions::ExecuteSolo(TArray<TWeakObjectPtr<UAtomSoundClass>> Objects) const
{
#if ENABLE_ATOM_DEBUG
	if (FAtomRuntimeManager* RuntimeManager = GCriWare->GetAtomRuntimeManager())
	{
		for (TWeakObjectPtr<UAtomSoundClass> ClassPtr : Objects)
		{
			if (UAtomSoundClass* Class = Cast<UAtomSoundClass>(ClassPtr.Get()))
			{
				RuntimeManager->GetDebugger().ToggleSoloSoundClass(Class->GetFName());
			}
		}
	}
#endif
}

bool FAtomSoundClassActions::IsActionCheckedMute(TArray<TWeakObjectPtr<UAtomSoundClass>> Objects) const
{
#if ENABLE_ATOM_DEBUG
	if (FAtomRuntimeManager* RuntimeManager = GCriWare->GetAtomRuntimeManager())
	{
		for (TWeakObjectPtr<UAtomSoundClass> ClassPtr : Objects)
		{
			if (UAtomSoundClass* Class = Cast<UAtomSoundClass>(ClassPtr.Get()))
			{
				if (RuntimeManager->GetDebugger().IsMuteSoundClass(Class->GetFName()))
				{
					return true;
				}
			}
		}
	}
#endif
	return false;
}

bool FAtomSoundClassActions::IsActionCheckedSolo(TArray<TWeakObjectPtr<UAtomSoundClass>> Objects) const
{
#if ENABLE_ATOM_DEBUG
	if (FAtomRuntimeManager* RuntimeManager = GCriWare->GetAtomRuntimeManager())
	{
		for (TWeakObjectPtr<UAtomSoundClass> ClassPtr : Objects)
		{
			if (UAtomSoundClass* Class = Cast<UAtomSoundClass>(ClassPtr.Get()))
			{
				if (RuntimeManager->GetDebugger().IsSoloSoundClass(Class->GetFName()))
				{
					return true;
				}
			}
		}
	}
#endif
	return false;
}

bool FAtomSoundClassActions::CanExecuteMuteCommand(TArray<TWeakObjectPtr<UAtomSoundClass>> Objects) const
{
#if ENABLE_ATOM_DEBUG
	if (FAtomRuntimeManager* RuntimeManager = GCriWare->GetAtomRuntimeManager())
	{
		// Allow muting if we're not Soloing.
		for (TWeakObjectPtr<UAtomSoundClass> ClassPtr : Objects)
		{
			if (UAtomSoundClass* Class = Cast<UAtomSoundClass>(ClassPtr.Get()))
			{
				if (RuntimeManager->GetDebugger().IsSoloSoundClass(Class->GetFName()))
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

bool FAtomSoundClassActions::CanExecuteSoloCommand(TArray<TWeakObjectPtr<UAtomSoundClass>> Objects) const
{
#if ENABLE_ATOM_DEBUG
	if (FAtomRuntimeManager* RuntimeManager = GCriWare->GetAtomRuntimeManager())
	{
		// Allow Soloing if we're not Muting.
		for (TWeakObjectPtr<UAtomSoundClass> ClassPtr : Objects)
		{
			if (UAtomSoundClass* Class = Cast<UAtomSoundClass>(ClassPtr.Get()))
			{
				if (RuntimeManager->GetDebugger().IsMuteSoundClass(Class->GetFName()))
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
