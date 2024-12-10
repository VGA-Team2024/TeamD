#include "Atom/AtomParameter.h"

#include "CriWareCorePrivate.h"

#define LOCTEXT_NAMESPACE "AtomParameters"

void FAtomParameter::Merge(const FAtomParameter& InParameter, bool bInTakeName, bool bInTakeType, bool bInMergeArrayTypes)
{

}

void FAtomParameter::Merge(TArray<FAtomParameter>&& InParams, TArray<FAtomParameter>& OutParams)
{

}

/*
 * FAtomSelectorParam struct
 *****************************************************************************/

FAtomSelectorParam::FAtomSelectorParam()
{
#if WITH_EDITOR
	/* Localization of unreal properties metadata with LOCTEXT markups and reflection */
	CRI_LOCSTRUCT(StaticStruct());
#endif
}

#undef LOCTEXT_NAMESPACE
