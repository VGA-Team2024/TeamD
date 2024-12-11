
#include "Atom/Gameplay/AtomAisacVolumeComponent.h"

#include "CriWareCorePrivate.h"
#include "CriWare.h"
#include "Atom/AtomVolume.h"
#include "Atom/AtomActiveSound.h"
#include "Atom/Gameplay/AtomGameplayFlags.h"

#define LOCTEXT_NAMESPACE "AtomAisacVolumeComponent"

constexpr TCHAR FAtomProxyMutator_Aisac::MutatorAisacName[];

FAtomProxyMutator_Aisac::FAtomProxyMutator_Aisac()
{
	MutatorName = MutatorAisacName;
}

void FAtomProxyMutator_Aisac::Apply(FAtomProxyActiveSoundParams& Params) const
{
	check(IsInAtomThread());

	// Determine location state.  Inside if any of the following conditions are met:
	// spatialization disabled on the active sound, we're in the same volume as the listener,
	// or if the active sound's interior settings are still 'default' (bUsingWorldSettings)
	EAtomVolumeLocationState LocationState = EAtomVolumeLocationState::OutsideTheVolume;
	if (Params.bListenerInVolume || !Params.bAllowSpatialization || Params.bUsingWorldSettings)
	{
		LocationState = EAtomVolumeLocationState::InsideTheVolume;
	}

	if (AisacControlSettings.Num() > 0)
	{
		for (const FAtomVolumeAisacControlSettings& ControlSetting : AisacControlSettings)
		{
			if (ControlSetting.ListenerLocationState == LocationState)
			{
				for (const FAtomAisacControlSettings& ControlInfo : ControlSetting.AisacControls)
				{
					Params.AisacControls.Add(ControlInfo);
				}
			}
		}
	}
}

UAtomAisacVolumeComponent::UAtomAisacVolumeComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PayloadType = AtomGameplay::EComponentPayload::AGCP_ActiveSound;
	bAutoActivate = true;

#if WITH_EDITOR
	// Localization of unreal properties metadata with LOCTEXT markups and reflection
	CRI_LOCCLASS(GetClass());
#endif // WITH_EDITOR
}

void UAtomAisacVolumeComponent::SetAisacControlSettings(const TArray<FAtomVolumeAisacControlSettings>& NewAisacControlSettings)
{
	AisacControlSettings = NewAisacControlSettings;

	// Let the parent volume know we've changed
	NotifyDataChanged();
}

TSharedPtr<FAtomProxyVolumeMutator> UAtomAisacVolumeComponent::FactoryMutator() const
{
	return MakeShared<FAtomProxyMutator_Aisac>();
}

void UAtomAisacVolumeComponent::CopyAtomDataToMutator(TSharedPtr<FAtomProxyVolumeMutator>& Mutator) const
{
	TSharedPtr<FAtomProxyMutator_Aisac> AisacMutator = StaticCastSharedPtr<FAtomProxyMutator_Aisac>(Mutator);
	AisacMutator->AisacControlSettings = AisacControlSettings;
}

#undef LOCTEXT_NAMESPACE
