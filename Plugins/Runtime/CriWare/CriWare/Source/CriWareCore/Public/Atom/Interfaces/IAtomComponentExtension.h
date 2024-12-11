#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/ScriptInterface.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"

#include "IAtomComponentExtension.generated.h"

/**
 * Interface to implement for generating safe extensions for synth and external components.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UAtomComponentExtensionInterface : public UInterface
{
	GENERATED_BODY()
};

class IAtomComponentExtensionInterface
{
	GENERATED_BODY()

public:

	/* Generates or gets the atom component extension. */
	virtual UAtomComponentExtension* GetComponentExtension() = 0;
};

/**
 * Base class that safely extend functions and properties accessiblity for synth and external components.
 * 
 */
UCLASS(abstract, BlueprintType, MinimalApi)
class UAtomComponentExtension : public UObject
{
	GENERATED_BODY()

public:

	virtual ~UAtomComponentExtension() = default;
};
