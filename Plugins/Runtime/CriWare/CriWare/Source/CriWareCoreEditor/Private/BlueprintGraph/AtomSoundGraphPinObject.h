
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include "AtomSoundGraphPinObject.generated.h"

// Forward Definitions
class UAtomSoundBase;

/** class to hold a Sound property to permit a Sound selection. (Local Use only) */
UCLASS(Transient)
class UAtomSoundGraphPinObject
	: public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(Transient, EditAnywhere, Category = Sound)
	TObjectPtr<class UAtomSoundBase> Sound;
};
