
#pragma once

#include "UObject/Interface.h"

#include "IAtomActiveSoundUpdate.generated.h"

// Forward Declarations 
struct FAtomActiveSound;
struct FAtomSoundParseParameters;

/** Interface for modifying Atom active sounds during their update */
UINTERFACE(meta = (CannotImplementInterfaceInBlueprint))
class UAtomActiveSoundUpdate : public UInterface
{
	GENERATED_BODY()
};

class IAtomActiveSoundUpdate
{
	GENERATED_BODY()

public:

	/**
	 * Gathers interior data that can affect the active sound. Non-const as this step can be used to track state about the sound on the implementing object.
	 *
	 * @param ActiveSound	The active sound affected.
	 * @param ParseParams	The parameters to apply to the playback instances.
	 */
	virtual void GatherInteriorData(const FAtomActiveSound& ActiveSound, FAtomSoundParseParameters& ParseParams) = 0;

	/**
	 * Applies interior data previously collected to the active sound and parse parameters.
	 *
	 * @param ActiveSound	The active sound affected.
	 * @param ParseParams	The parameters to apply to the playback instances.
	 */
	virtual void ApplyInteriorSettings(const FAtomActiveSound& ActiveSound, FAtomSoundParseParameters& ParseParams) = 0;

	/**
	 * Called when the active sound is being removed from the Atom runtime.
	 *
	 * @param ActiveSound	The active sound.
	 */
	virtual void OnNotifyPendingDelete(const FAtomActiveSound& ActiveSound) = 0;
};
