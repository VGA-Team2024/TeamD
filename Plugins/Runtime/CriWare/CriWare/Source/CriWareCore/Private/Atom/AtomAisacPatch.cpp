
#include "Atom/AtomAisacPatch.h"

#include "Atom/AtomConfig.h"

UAtomConfig* UAtomAisacPatch::GetConfig() const
{
	return Cast<UAtomConfig>(GetOuter());
}
