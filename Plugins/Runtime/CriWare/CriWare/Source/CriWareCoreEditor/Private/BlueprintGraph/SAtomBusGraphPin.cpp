
#include "SAtomBusGraphPin.h"

#include "PropertyEditorModule.h"
#include "Widgets/Text/STextBlock.h"

#include "Widgets/SCustomizedObjectCombo.h"
#include "Atom/AtomBus.h"
#include "AtomBusGraphPinObject.h"

#define LOCTEXT_NAMESPACE "SAtomBusGraphPin"

void SAtomBusGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);	
}

TSharedRef<SWidget> SAtomBusGraphPin::GetDefaultValueWidget()
{
	if (!BusPropertyHolderObject)
	{
		BusPropertyHolderObject = NewObject<UAtomBusGraphPinObject>();
	}
	BusPropertyHolderObject->Bus = Cast<UAtomBus>(GraphPinObj->DefaultObject);

	//Create widget
	return SNew(SVerticalBox)
		.IsEnabled(this, &SGraphPin::GetDefaultValueIsEditable)
		.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
		+ SVerticalBox::Slot()
		[
			SNew(SCustomizedObjectCombo<UAtomBus>)
			.DisplayThumbnail(false)
			.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
			//.Filter(FilterString)
			.TargetObject(this, &SAtomBusGraphPin::GetValue)
			.Object(BusPropertyHolderObject)
			.OnSelectionChanged(this, &SAtomBusGraphPin::OnSelectionChanged)
		];
}

void SAtomBusGraphPin::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(BusPropertyHolderObject);
}

bool SAtomBusGraphPin::OnSelectionChanged(UObject* BusObject)
{
	if (UAtomBus* Bus = Cast<UAtomBus>(BusObject))
	{
		BusPropertyHolderObject->Bus = Bus;
	}

	return SetValue(BusPropertyHolderObject->Bus) && BusPropertyHolderObject->Bus != nullptr;
}

bool SAtomBusGraphPin::SetValue(UAtomBus* Bus)
{
	if (GraphPinObj->DefaultObject != Bus)
	{
		GraphPinObj->GetSchema()->TrySetDefaultObject(*GraphPinObj, Bus);
		return true;
	}

	return false;
}

UAtomBus* SAtomBusGraphPin::GetValue() const
{
	return Cast<UAtomBus>(GraphPinObj->DefaultObject);
}

#undef LOCTEXT_NAMESPACE
