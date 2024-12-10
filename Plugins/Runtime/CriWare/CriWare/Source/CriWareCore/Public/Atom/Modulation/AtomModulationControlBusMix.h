
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UnrealType.h"

#include "AtomModulation.h"
#include "AtomModulationValue.h"

#include "AtomModulationControlBusMix.generated.h"

// Forward Declarations
class UAtomModulationControlBus;

USTRUCT(BlueprintType)
struct CRIWARECORE_API FAtomModulationControlBusMixStage
{
	GENERATED_BODY()

public:

	FAtomModulationControlBusMixStage();
	FAtomModulationControlBusMixStage(UAtomModulationControlBus* InBus, const float TargetValue);

	/* Bus controlled by stage. */
	UPROPERTY(EditAnywhere, Category = Stage, BlueprintReadWrite)
	TObjectPtr<UAtomModulationControlBus> Bus;

	/* Value mix is set to. */
	UPROPERTY(EditAnywhere, Category = Stage, BlueprintReadWrite)
	FAtomModulationMixValue Value;
};

UCLASS(config = Engine, AutoExpandCategories = (Stage, Mix), EditInLineNew, BlueprintType, MinimalAPI)
class UAtomModulationControlBusMix : public UObject
{
	GENERATED_BODY()

public:

	UAtomModulationControlBusMix(const FObjectInitializer& ObjectInitializer);

protected:
	// Loads the mix from the provided profile index
	//UFUNCTION(Category = Mix, meta = (CallInEditor = "true"))
	//void LoadMixFromProfile();

	// Saves the mix to the provided profile index
	//UFUNCTION(Category = Mix, meta = (CallInEditor = "true"))
	//void SaveMixToProfile();

	// Solos this mix, deactivating all others and activating this
	// (if its not already active), while testing in-editor in all
	// active worlds
	UFUNCTION(Category = Mix, meta = (CallInEditor = "true"))
	void SoloMix();

	// Activates this mix in all active worlds
	UFUNCTION(Category = Mix, meta = (CallInEditor = "true"))
	void ActivateMix();

	// Deactivates this mix in all active worlds
	UFUNCTION(Category = Mix, meta = (CallInEditor = "true"))
	void DeactivateMix();

	// Deactivates all mixes in all active worlds
	UFUNCTION(Category = Mix, meta = (CallInEditor = "true"))
	void DeactivateAllMixes();

public:
	UPROPERTY(EditAnywhere, Transient, Category = Mix)
	uint32 ProfileIndex;

	virtual void BeginDestroy() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& InPropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& InPropertyChangedEvent) override;
	virtual void OnPropertyChanged(FProperty* Property, EPropertyChangeType::Type ChangeType);
#endif // WITH_EDITOR

	/* Array of stages controlled by mix. */
	UPROPERTY(EditAnywhere, Category = Mix, BlueprintReadOnly)
	TArray<FAtomModulationControlBusMixStage> MixStages;
};
