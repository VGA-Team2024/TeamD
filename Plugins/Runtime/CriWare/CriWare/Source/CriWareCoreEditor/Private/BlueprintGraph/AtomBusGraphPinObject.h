
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include "AtomBusGraphPinObject.generated.h"

// Forward Definitions
class UAtomBus;

/** class to hold an AtomBus property to permit a Bus selection. (Local use only) */
UCLASS(Transient)
class UAtomBusGraphPinObject
	: public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(Transient, EditAnywhere, Category = Atom)
	TObjectPtr<class UAtomBus> Bus;
};
