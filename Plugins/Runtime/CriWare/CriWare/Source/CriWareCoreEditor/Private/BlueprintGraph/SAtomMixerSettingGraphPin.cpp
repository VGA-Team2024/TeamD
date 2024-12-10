
#include "SAtomMixerSettingGraphPin.h"

#include "PropertyEditorModule.h"
#include "Widgets/Text/STextBlock.h"

#include "Widgets/SCustomizedObjectCombo.h"
#include "Atom/AtomConfig.h"
#include "AtomMixerSettingGraphPinObject.h"

#define LOCTEXT_NAMESPACE "SAtomMixerSettingGraphPin"

void SAtomMixerSettingGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);	
}

TSharedRef<SWidget> SAtomMixerSettingGraphPin::GetDefaultValueWidget()
{
	if (!DspBusSettingPropertyHolderObject)
	{
		DspBusSettingPropertyHolderObject = NewObject<UAtomMixerSettingGraphPinObject>();
	}
	DspBusSettingPropertyHolderObject->DspBusSetting = Cast<UAtomDspBusSetting>(GraphPinObj->DefaultObject);

	//Create widget
	return SNew(SVerticalBox)
		.IsEnabled(this, &SGraphPin::GetDefaultValueIsEditable)
		.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
		+ SVerticalBox::Slot()
		[
			SNew(SCustomizedObjectCombo<UAtomDspBusSetting>)
			.DisplayThumbnail(false)
			.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
			//.Filter(FilterString)
			.TargetObject(this, &SAtomMixerSettingGraphPin::GetValue)
			.Object(DspBusSettingPropertyHolderObject)
			.OnSelectionChanged(this, &SAtomMixerSettingGraphPin::OnSelectionChanged)
		];
}

void SAtomMixerSettingGraphPin::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(DspBusSettingPropertyHolderObject);
}

bool SAtomMixerSettingGraphPin::OnSelectionChanged(UObject* DspBusSettingObject)
{
	if (UAtomDspBusSetting* DspBusSetting = Cast<UAtomDspBusSetting>(DspBusSettingObject))
	{
		DspBusSettingPropertyHolderObject->DspBusSetting = DspBusSetting;
	}

	return SetValue(DspBusSettingPropertyHolderObject->DspBusSetting) && DspBusSettingPropertyHolderObject->DspBusSetting != nullptr;
}

bool SAtomMixerSettingGraphPin::SetValue(UAtomDspBusSetting* DspBusSetting)
{
	if (GraphPinObj->DefaultObject != DspBusSetting)
	{
		GraphPinObj->GetSchema()->TrySetDefaultObject(*GraphPinObj, DspBusSetting);
		return true;
	}

	return false;
}

UAtomDspBusSetting* SAtomMixerSettingGraphPin::GetValue() const
{
	return Cast<UAtomDspBusSetting>(GraphPinObj->DefaultObject);
}

#undef LOCTEXT_NAMESPACE
