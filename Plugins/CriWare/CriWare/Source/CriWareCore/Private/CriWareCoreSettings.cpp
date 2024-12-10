/****************************************************************************
*
* CRI Middleware SDK
*
* Copyright (c) 2021 CRI Middleware Co., Ltd.
*
* Library  : CRIWARE plugin for Unreal Engine 4
* Module   : Module Settings for CriWare
* File     : CriWareCoreSettings.cpp
*
****************************************************************************/

#include "CriWareCoreSettings.h"

#if WITH_EDITOR
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Runtime/Launch/Resources/Version.h"
#endif

#include "CriWareCorePrivate.h"
#include "CriWare.h"
#include "Atom/AtomRuntime.h"
#include "Atom/AtomConfig.h"
#include "Atom/AtomRack.h"
#include "Atom/Modulation/AtomModulationParameter.h"

#define LOCTEXT_NAMESPACE "CriWareCoreSettings"

/* VoicesSettings functions
 *****************************************************************************/

void FCriWareStandardVoicesSettings::OverridesWith(const FCriWareStandardVoicesSettings& Settings)
{
	if (Settings.NumStandardMemoryVoices > 0)
		NumStandardMemoryVoices = Settings.NumStandardMemoryVoices;
	if (Settings.StandardMemoryVoiceNumChannels > 0)
		StandardMemoryVoiceNumChannels = Settings.StandardMemoryVoiceNumChannels;
	if (Settings.StandardMemoryVoiceSamplingRate > 0)
		StandardMemoryVoiceSamplingRate = Settings.StandardMemoryVoiceSamplingRate;
	if (Settings.NumStandardStreamingVoices > 0)
		NumStandardMemoryVoices = Settings.NumStandardStreamingVoices;
	if (Settings.StandardStreamingVoiceNumChannels > 0)
		StandardStreamingVoiceNumChannels = Settings.StandardStreamingVoiceNumChannels;
	if (Settings.StandardStreamingVoiceSamplingRate > 0)
		StandardStreamingVoiceSamplingRate = Settings.StandardStreamingVoiceSamplingRate;
}

void FCriWareHcaMxVoicesSettings::OverridesWith(const FCriWareHcaMxVoicesSettings& Settings)
{
	if (Settings.NumHcaMxMemoryVoices > 0)
		NumHcaMxMemoryVoices = Settings.NumHcaMxMemoryVoices;
	if (Settings.HcaMxMemoryVoiceNumChannels > 0)
		HcaMxMemoryVoiceNumChannels = Settings.HcaMxMemoryVoiceNumChannels;
	if (Settings.NumHcaMxStreamingVoices > 0)
		NumHcaMxStreamingVoices = Settings.NumHcaMxStreamingVoices;
	if (Settings.HcaMxStreamingVoiceNumChannels > 0)
		HcaMxStreamingVoiceNumChannels = Settings.HcaMxStreamingVoiceNumChannels;
	if (Settings.HcaMxVoiceSamplingRate > 0)
		HcaMxVoiceSamplingRate = Settings.HcaMxVoiceSamplingRate;
}

/* UCriWareCoreSettings structors
 *****************************************************************************/

UCriWareCoreSettings::UCriWareCoreSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SectionName = TEXT("Core");

	// File System
	NumBinders = 64;
	MaxBindings = 64;
	NumLoaders = 64;
	bEnableLoggingFileSystem = false;

	// Atom
	AtomConfig = nullptr;
	bUseInGamePreview = false;
	MonitorCommunicationBufferSize = 2 * 1024 * 1024; // 2MB
	MaxPitch = 2400.0f;
	DistanceFactor = 0.01f; // meter
	MaxSoundSources = 64;
	NumReservedSoundSources = 8;
	bEnableLoggingAtom = false;

	// Atom - Sound Renderer
	bEnableBinauralSpatialization = false;
	bUseAudioLink = false;
	bUseUnrealSoundRenderer = false;

	// Atom - Voices
	bUseAutomaticVoiceManagement = false;
	//     Common Standard
	StandardVoicesSettings.NumStandardMemoryVoices = 32;
	StandardVoicesSettings.StandardMemoryVoiceNumChannels = CRIATOM_DEFAULT_INPUT_MAX_CHANNELS;
	StandardVoicesSettings.StandardMemoryVoiceSamplingRate = CRIATOM_DEFAULT_INPUT_MAX_SAMPLING_RATE;
	StandardVoicesSettings.NumStandardStreamingVoices = 32;
	StandardVoicesSettings.StandardStreamingVoiceNumChannels = CRIATOM_DEFAULT_INPUT_MAX_CHANNELS;
	StandardVoicesSettings.StandardStreamingVoiceSamplingRate = CRIATOM_DEFAULT_INPUT_MAX_SAMPLING_RATE;
	//     Common HCA-MX
	HcaMxVoicesSettings.NumHcaMxMemoryVoices = 0;
	HcaMxVoicesSettings.HcaMxMemoryVoiceNumChannels = CRIATOM_DEFAULT_INPUT_MAX_CHANNELS;
	HcaMxVoicesSettings.NumHcaMxStreamingVoices = 0;
	HcaMxVoicesSettings.HcaMxStreamingVoiceNumChannels = CRIATOM_DEFAULT_INPUT_MAX_CHANNELS;
	HcaMxVoicesSettings.HcaMxVoiceSamplingRate = CRIATOM_DEFAULT_OUTPUT_SAMPLING_RATE;

	// Hides Mana class settings for LE
#ifndef CRIWARE_UE_LE
	bCanEditManaClassName = true;
#else
	bCanEditManaClassName = false;
#endif
}

//~ UDeveloperSettings interface
#if WITH_EDITOR
FText UCriWareCoreSettings::GetSectionText() const
{ 
	return LOCTEXT("CriWareCoreSettingsSectionText", "Core");
}

FText UCriWareCoreSettings::GetSectionDescription() const
{
	return LOCTEXT("CriWareCoreSettingsSectionDescription", "Settings for CriWare Core");
}
#endif

//~ UObject interface
void UCriWareCoreSettings::PostInitProperties()
{
	Super::PostInitProperties();

	if (MaxSoundPlaybackHandles_DEPRECATED > 0)
	{
		MaxSoundSources = MaxSoundPlaybackHandles_DEPRECATED;
		NumReservedSoundSources = 0;

		MaxSoundPlaybackHandles_DEPRECATED = 0;
		TryUpdateDefaultConfigFile();
	}
}

#if WITH_EDITOR
void UCriWareCoreSettings::PreEditChange(FProperty* PropertyAboutToChange)
{
	// Cache master rack in case user tries to set to rack that isn't a top-level rack
	CachedMasterRack = MasterRack;

	// Cache master rack in case user tries to set an invalid rack that cannot be used to spatialize sounds.
	CachedSpatializationRack = SpatializationRack;

	// Cache Atom configuration in case user cancels the operation from the default DSP bus setting message box
	CachedAtomConfig = AtomConfig;
}

void UCriWareCoreSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (GEditor && GEditor->IsPlayingSessionInEditor())
	{
		UE_LOG(LogCriWareAtom, Warning, TEXT("Parameter changing is not applied in PIE Mode."));
		return;
	}

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	const FName MemberPropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCriWareCoreSettings, MasterRack))
	{
		if (UAtomRack* NewAtomRack = Cast<UAtomRack>(MasterRack.TryLoad()))
		{
			if (NewAtomRack->ParentRack)
			{
				FNotificationInfo Info(LOCTEXT("CriWareCore_Settings_InvalidMasterRack", "'Master Rack' cannot be set to submix with parent."));
				Info.bFireAndForget = true;
				Info.ExpireDuration = 2.0f;
				Info.bUseThrobber = true;
				FSlateNotificationManager::Get().AddNotification(Info);

				// revert
				MasterRack = CachedMasterRack;
			}
			else
			{
				// reactivate the active Atom runtime
				ReactivateActiveRuntime();
			}
		}
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCriWareCoreSettings, SpatializationRack))
	{
		if (UAtomRackBase* NewAtomRack = Cast<UAtomRackBase>(SpatializationRack.TryLoad()))
		{
			// reactivate the active Atom runtime
			ReactivateActiveRuntime();
		}
	}

	// setup atom configuration on change
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCriWareCoreSettings, AtomConfig))
	{
		auto AtomConfigAsset = Cast<UAtomConfig>(AtomConfig.TryLoad());
		auto MasterRackAsset = Cast<UAtomRack>(MasterRack.TryLoad());

		bool bWasCanceled = false;
		bool bConfigToMaster = ShowApplyAtomConfigToMasterRackDialog(AtomConfigAsset, bWasCanceled);
		if (bWasCanceled)
		{
			// revert
			AtomConfig = CachedAtomConfig;
		}
		else
		{
			// Apply to Atom runtime
			if (GCriWare)
			{
				FAtomRuntime* ActiveAtomRuntime = nullptr;
				if (FAtomRuntimeHandle RuntimeHandle = GCriWare->GetActiveAtomRuntime())
				{
					ActiveAtomRuntime = RuntimeHandle.GetAtomRuntime();
					if (ActiveAtomRuntime)
					{
						ActiveAtomRuntime->Deactivate();
					}
				}

				if (!GCriWare->SetAtomConfiguration(AtomConfigAsset))
				{
					FNotificationInfo Info(LOCTEXT("CriWareCore_Settings_InvalidAtomConfig", "Failed to register AtomConfig to CriWare libary."));
					Info.bFireAndForget = true;
					Info.ExpireDuration = 2.0f;
					Info.bUseThrobber = true;
					FSlateNotificationManager::Get().AddNotification(Info);
				}
				else if (bConfigToMaster)
				{
					// Apply Dsp bus setting to master rack
					if (MasterRackAsset)
					{
						auto DefaultDspBusSetting = AtomConfigAsset ? AtomConfigAsset->GetDefaultDspBusSetting() : nullptr;
						MasterRackAsset->SetDspBusSetting(DefaultDspBusSetting, true);
						MasterRackAsset->MarkPackageDirty();
					}
				}

				if (ActiveAtomRuntime)
				{
					// reactivate the active Atom runtime
					ActiveAtomRuntime->Activate();
				}
			}
		}
	}

	// constants if changed that need to restart Atom runtime to apply
	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet
		&& (PropertyName == GET_MEMBER_NAME_CHECKED(UCriWareCoreSettings, NumBinders)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UCriWareCoreSettings, MaxBindings)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UCriWareCoreSettings, NumLoaders)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UCriWareCoreSettings, bEnableLoggingFileSystem)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UCriWareCoreSettings, MaxPitch)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UCriWareCoreSettings, MaxSoundSources)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UCriWareCoreSettings, NumReservedSoundSources)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UCriWareCoreSettings, bEnableLoggingAtom)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UCriWareCoreSettings, bEnableBinauralSpatialization)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UCriWareCoreSettings, DefaultOutputSubmix)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UCriWareCoreSettings, bUseAutomaticVoiceManagement)
		|| MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCriWareCoreSettings, StandardVoicesSettings)
		|| MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCriWareCoreSettings, HcaMxVoicesSettings)))
	{
		// reactivate the active Atom runtime
		ReactivateActiveRuntime();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);

	CriWareCoreSettingsChanged.Broadcast();
}

void UCriWareCoreSettings::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property)
	{
		FName PropertyName = PropertyChangedEvent.Property->GetFName();

		LoadDefaultObjects();

		if (PropertyName == GET_MEMBER_NAME_CHECKED(UCriWareCoreSettings, ModulationParameters))
		{
			UE_LOG(LogCriWareAtom, Display, TEXT("Reloading Parameters from CriWare Atom Settings..."));
			RegisterModulationParameters();
		}
	}

	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

void UCriWareCoreSettings::ReactivateActiveRuntime() const
{
	// reactivate the active Atom runtime
	if (GCriWare)
	{
		if (FAtomRuntimeHandle RuntimeHandle = GCriWare->GetActiveAtomRuntime())
		{
			if (FAtomRuntime* AtomRuntime = RuntimeHandle.GetAtomRuntime())
			{
				AtomRuntime->Deactivate();
				AtomRuntime->Activate();
			}
		}
	}
}

bool UCriWareCoreSettings::ShowApplyAtomConfigToMasterRackDialog(const UAtomConfig* InAtomConfig, bool& bOutWasCanceled)
{
	auto MasterRackAsset = Cast<UAtomRack>(MasterRack.TryLoad());
	if (!MasterRackAsset)
	{
		return false;
	}

	if (InAtomConfig)
	{
		// Ask user if master rack need to use the default DSP bus settings from ACF data.
		const UAtomDspBusSetting* DefaultSetting = InAtomConfig->GetDefaultDspBusSetting();
		if (DefaultSetting)
		{
			FText MessageBoxTitle = LOCTEXT("AtomConfigDspSettingTitle", "Setup Atom Configuration File");
			FText MessageBoxContent = FText::Format(LOCTEXT("AtomConfigDspSettingContent",
				"Atom configuration file contains DSP bus setting '{0}' marked as default.\n\n"
				"Do you want to update Master Rack with default DSP bus setting?\n\n"
				"Warning: All buses and their current settings will be reset."), FText::FromString(DefaultSetting->GetSettingName()));

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
			auto Result = FMessageDialog::Open(EAppMsgType::YesNoCancel, MessageBoxContent, MessageBoxTitle);
#else
			auto Result = FMessageDialog::Open(EAppMsgType::YesNoCancel, MessageBoxContent, &MessageBoxTitle);
#endif
			bool bCancelByUser = Result == EAppReturnType::Type::Cancel;
			bool bSetupToMaster = Result == EAppReturnType::Type::Yes;

			if (bCancelByUser)
			{
				bOutWasCanceled = true;
				return false;
			}

			if (!bSetupToMaster)
			{
				return false;
			}
		}

		// ACF should always have a default settings
	}
	else
	{
		if (!MasterRackAsset->DspBusSetting)
		{
			return false;
		}

		FText MessageBoxTitle = LOCTEXT("AtomConfigDspSettingTitle", "Setup Atom Configuration File");
		FText MessageBoxContent = LOCTEXT("RemoveAtomConfigContent",
			"Atom Configuration file will be removed.\n\n"
			"Do you want to remove Master Rack Dsp bus setting?\n\n"
			"Warning: All buses and their current settings will be reset.");

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
		auto Result = FMessageDialog::Open(EAppMsgType::YesNoCancel, MessageBoxContent, MessageBoxTitle);
#else
		auto Result = FMessageDialog::Open(EAppMsgType::YesNoCancel, MessageBoxContent, &MessageBoxTitle);
#endif
		bool bCancelByUser = Result == EAppReturnType::Type::Cancel;
		bool bSetupToMaster = Result == EAppReturnType::Type::Yes;

		if (bCancelByUser)
		{
			bOutWasCanceled = true;
			return false;
		}

		if (!bSetupToMaster)
		{
			return false;
		}
	}

	return true;
}
#endif

void UCriWareCoreSettings::LoadDefaultObjects()
{
	UE_LOG(LogCriWareAtom, Display, TEXT("Loading Default Audio Settings Objects..."));

	// TODO: Move all soft object paths to load here (SoundMixes, Submixes, etc.)
	//static const FString EngineSoundsDir = TEXT("/Engine/EngineSounds");

	if (DefaultSoundClass)
	{
		DefaultSoundClass->RemoveFromRoot();
		DefaultSoundClass = nullptr;
	}

	if (UObject* SoundClassObject = DefaultSoundClassName.TryLoad())
	{
		DefaultSoundClass = CastChecked<UAtomSoundClass>(SoundClassObject);
		DefaultSoundClass->AddToRoot();
	}

#if WITH_EDITOR
	if (!DefaultSoundClass)
	{
		DefaultSoundClassName = CachedSoundClass;
		if (UObject* SoundClassObject = DefaultSoundClassName.TryLoad())
		{
			DefaultSoundClass = CastChecked<UAtomSoundClass>(SoundClassObject);
			DefaultSoundClass->AddToRoot();
		}
	}
#endif // WITH_EDITOR

	/*if (!DefaultSoundClass)
	{
		UE_LOG(LogCriWareAtom, Warning, TEXT("Failed to load Default SoundClassObject from path '%s'.  Attempting to fall back to engine default."), *DefaultSoundClassName.GetAssetPathString());
		DefaultSoundClassName.SetPath(EngineSoundsDir / TEXT("Master"));
		if (UObject* SoundClassObject = DefaultSoundClassName.TryLoad())
		{
			DefaultSoundClass = CastChecked<UAtomSoundClass>(SoundClassObject);
			DefaultSoundClass->AddToRoot();
		}
	}

	if (!DefaultSoundClass)
	{
		UE_LOG(LogCriWareAtom, Error, TEXT("Failed to load Default SoundClassObject from path '%s'."), *DefaultSoundClassName.GetAssetPathString());
	}*/

	if (DefaultManaSoundClass)
	{
		DefaultManaSoundClass->RemoveFromRoot();
		DefaultManaSoundClass = nullptr;
	}

	if (UObject* ManaSoundClassObject = DefaultManaSoundClassName.TryLoad())
	{
		DefaultManaSoundClass = CastChecked<UAtomSoundClass>(ManaSoundClassObject);
		DefaultManaSoundClass->AddToRoot();
	}
	else
	{
		UE_LOG(LogAudio, Display, TEXT("No default ManaSoundClassObject specified (or failed to load)."));
	}

	if (DefaultSoundConcurrency)
	{
		DefaultSoundConcurrency->RemoveFromRoot();
		DefaultSoundConcurrency = nullptr;
	}

	if (UObject* SoundConcurrencyObject = DefaultSoundConcurrencyName.TryLoad())
	{
		DefaultSoundConcurrency = CastChecked<UAtomConcurrency>(SoundConcurrencyObject);
		DefaultSoundConcurrency->AddToRoot();
	}
	else
	{
		UE_LOG(LogCriWareAtom, Display, TEXT("No default SoundConcurrencyObject specified (or failed to load)."));
	}
}

void UCriWareCoreSettings::RegisterModulationParameters() const
{
	Atom::UnregisterAllModulationParameters();

	for (const FSoftObjectPath& Path : ModulationParameters)
	{
		if (UAtomModulationParameter* Param = Cast<UAtomModulationParameter>(Path.TryLoad()))
		{
			FAtomModulationParameter NewParam = Param->CreateParameter();
			Atom::RegisterModulationParameter(NewParam.ParameterName, MoveTemp(NewParam));
			UE_LOG(LogCriWareAtom, Display, TEXT("Initialized Atom Modulation Parameter '%s'"), *NewParam.ParameterName.ToString());
		}
		else
		{
			UE_LOG(LogCriWareAtom, Error, TEXT("Failed to load parameter at '%s': Missing asset or invalid type."), *Path.GetAssetName());
		}
	}
}

UAtomSoundClass* UCriWareCoreSettings::GetDefaultSoundClass() const
{
	return DefaultSoundClass;
}

UAtomSoundClass* UCriWareCoreSettings::GetDefaultManaSoundClass() const
{
	return DefaultManaSoundClass;
}

UAtomConcurrency* UCriWareCoreSettings::GetDefaultSoundConcurrency() const
{
	return DefaultSoundConcurrency;
}

#undef LOCTEXT_NAMESPACE
