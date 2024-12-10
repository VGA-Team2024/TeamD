
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include "AtomMixerSettingGraphPinObject.generated.h"

// Forward Definitions
class UAtomBus;

/** class to hold an UAtomDspBusSetting property to permit a setting selection. (Local use only) */
UCLASS(Transient)
class UAtomMixerSettingGraphPinObject
	: public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(Transient, EditAnywhere, Category = Atom)
	TObjectPtr<class UAtomDspBusSetting> DspBusSetting;
};
