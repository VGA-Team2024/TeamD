
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include "AtomAisacPatchGraphPinObject.generated.h"

// Forward Definitions
class UAtomAisacPatch;

/** class to hold an AtomAisacPatch property to permit a AisacPatch selection. (Local use only) */
UCLASS(Transient)
class UAtomAisacPatchGraphPinObject
	: public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(Transient, EditAnywhere, Category = Atom)
	TObjectPtr<class UAtomAisacPatch> AisacPatch;
};
