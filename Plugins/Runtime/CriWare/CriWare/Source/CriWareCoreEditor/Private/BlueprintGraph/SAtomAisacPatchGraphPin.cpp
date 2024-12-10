
#include "SAtomAisacPatchGraphPin.h"

#include "PropertyEditorModule.h"
#include "Widgets/Text/STextBlock.h"

#include "Widgets/SCustomizedObjectCombo.h"
#include "Atom/AtomAisacPatch.h"
#include "AtomAisacPatchGraphPinObject.h"

#define LOCTEXT_NAMESPACE "SAtomAisacPatchGraphPin"

void SAtomAisacPatchGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);	
}

TSharedRef<SWidget> SAtomAisacPatchGraphPin::GetDefaultValueWidget()
{
	if (!AisacPatchPropertyHolderObject)
	{
		AisacPatchPropertyHolderObject = NewObject<UAtomAisacPatchGraphPinObject>();
	}
	AisacPatchPropertyHolderObject->AisacPatch = Cast<UAtomAisacPatch>(GraphPinObj->DefaultObject);

	//Create widget
	return SNew(SVerticalBox)
		.IsEnabled(this, &SGraphPin::GetDefaultValueIsEditable)
		.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
		+ SVerticalBox::Slot()
		[
			SNew(SCustomizedObjectCombo<UAtomAisacPatch>)
			.DisplayThumbnail(false)
			.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
			//.Filter(FilterString)
			.TargetObject(this, &SAtomAisacPatchGraphPin::GetValue)
			.Object(AisacPatchPropertyHolderObject)
			.OnSelectionChanged(this, &SAtomAisacPatchGraphPin::OnSelectionChanged)
		];
}

void SAtomAisacPatchGraphPin::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(AisacPatchPropertyHolderObject);
}

bool SAtomAisacPatchGraphPin::OnSelectionChanged(UObject* AisacPatchObject)
{
	if (UAtomAisacPatch* AisacPatch = Cast<UAtomAisacPatch>(AisacPatchObject))
	{
		AisacPatchPropertyHolderObject->AisacPatch = AisacPatch;
	}

	return SetValue(AisacPatchPropertyHolderObject->AisacPatch) && AisacPatchPropertyHolderObject->AisacPatch != nullptr;
}

bool SAtomAisacPatchGraphPin::SetValue(UAtomAisacPatch* AisacPatch)
{
	if (GraphPinObj->DefaultObject != AisacPatch)
	{
		GraphPinObj->GetSchema()->TrySetDefaultObject(*GraphPinObj, AisacPatch);
		return true;
	}

	return false;
}

UAtomAisacPatch* SAtomAisacPatchGraphPin::GetValue() const
{
	return Cast<UAtomAisacPatch>(GraphPinObj->DefaultObject);
}

#undef LOCTEXT_NAMESPACE
