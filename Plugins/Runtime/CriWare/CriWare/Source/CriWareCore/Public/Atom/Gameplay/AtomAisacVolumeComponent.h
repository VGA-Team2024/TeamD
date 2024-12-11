
#pragma once

#include "AtomGameplayVolumeMutator.h"
#include "Atom/AtomAisacPatch.h"
#include "Atom/AtomVolume.h"

#include "AtomAisacVolumeComponent.generated.h"

/** Struct to determine dynamic Aisac Control data for use with Atom gameplay volumes. */
USTRUCT(BlueprintType)
struct CRIWARECORE_API FAtomVolumeAisacControlSettings
{
	GENERATED_BODY()

public:

	/** The state the listener needs to be in, relative to the Atom volume, for this bus send list to be used for a given sound. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atom Volume Bus Sends")
	EAtomVolumeLocationState ListenerLocationState = EAtomVolumeLocationState::InsideTheVolume;

	/** AISAC control array for sounds that are in the AtomListenerLocationState at the same time as the listener. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AISAC Controls")
	TArray<FAtomAisacControlSettings> AisacControls;
};

/**
 *  FAtomProxyMutator_Aisac - An Atom thread representation of Aisac.
 */
class FAtomProxyMutator_Aisac
	: public FAtomProxyVolumeMutator
{
public:

	FAtomProxyMutator_Aisac();
	virtual ~FAtomProxyMutator_Aisac() = default;

	TArray<FAtomVolumeAisacControlSettings> AisacControlSettings;

	virtual void Apply(FAtomProxyActiveSoundParams& Params) const override;

protected:

	constexpr static const TCHAR MutatorAisacName[] = TEXT("AISAC");
};

/**
 *  UAtomAisacVolumeComponent - Atom Gameplay Volume component for bus sends.
 */
UCLASS(Blueprintable, Config = Game, ClassGroup = ("AtomGameplayVolume"), meta = (BlueprintSpawnableComponent, DisplayName = "AISAC"))
class CRIWARECORE_API UAtomAisacVolumeComponent
	: public UAtomGameplayVolumeMutator
{
	GENERATED_BODY()

public:

	UAtomAisacVolumeComponent(const FObjectInitializer& ObjectInitializer);
	virtual ~UAtomAisacVolumeComponent() = default;

	UFUNCTION(BlueprintCallable, Category = "AtomGameplay")
	void SetAisacControlSettings(const TArray<FAtomVolumeAisacControlSettings>& NewAisacControlSettings);

	const TArray<FAtomVolumeAisacControlSettings>& GetAisacControlSettings() const { return AisacControlSettings; }

private:

	//~ Begin UAtomGameplayVolumeMutator Interface
	virtual TSharedPtr<FAtomProxyVolumeMutator> FactoryMutator() const override;
	virtual void CopyAtomDataToMutator(TSharedPtr<FAtomProxyVolumeMutator>& Mutator) const override;
	//~ End UAtomGameplayVolumeMutator Interface

	/** AISAC Controls to use for this component. Allows to apply AISAC controls dynamically to ausio on source and listener locations (relative to parent volume.) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Submixes, meta = (AllowPrivateAccess = "true"))
	TArray<FAtomVolumeAisacControlSettings> AisacControlSettings;
};
