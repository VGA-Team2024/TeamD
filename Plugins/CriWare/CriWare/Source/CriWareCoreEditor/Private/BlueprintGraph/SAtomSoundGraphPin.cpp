
#include "SAtomSoundGraphPin.h"

#include "PropertyEditorModule.h"
#include "Widgets/Text/STextBlock.h"

#include "Widgets/SCustomizedObjectCombo.h"
#include "Atom/AtomSoundBase.h"
#include "AtomSoundGraphPinObject.h"

#define LOCTEXT_NAMESPACE "SAtomSoundGraphPin"

void SAtomSoundGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);	
}

TSharedRef<SWidget> SAtomSoundGraphPin::GetDefaultValueWidget()
{
	if (!SoundPropertyHolderObject)
	{
		SoundPropertyHolderObject = NewObject<UAtomSoundGraphPinObject>();
	}
	SoundPropertyHolderObject->Sound = Cast<UAtomSoundBase>(GraphPinObj->DefaultObject);

	//Create widget
	return SNew(SVerticalBox)
		.IsEnabled(this, &SGraphPin::GetDefaultValueIsEditable)
		.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
		+ SVerticalBox::Slot()
		[
			SNew(SCustomizedObjectCombo<UAtomSoundBase>)
			.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
			//.Filter(FilterString)
			.TargetObject(this, &SAtomSoundGraphPin::GetValue)
			.Object(SoundPropertyHolderObject)
			.OnSelectionChanged(this, &SAtomSoundGraphPin::OnSelectionChanged)
		];
}

void SAtomSoundGraphPin::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(SoundPropertyHolderObject);
}

bool SAtomSoundGraphPin::OnSelectionChanged(UObject* SoundObject)
{
	if (UAtomSoundBase* Sound = Cast<UAtomSoundBase>(SoundObject))
	{
		SoundPropertyHolderObject->Sound = Sound;
	}

	return SetValue(SoundPropertyHolderObject->Sound) && SoundPropertyHolderObject->Sound != nullptr;
}

bool SAtomSoundGraphPin::SetValue(UAtomSoundBase* Sound)
{
	if (GraphPinObj->DefaultObject != Sound)
	{
		GraphPinObj->GetSchema()->TrySetDefaultObject(*GraphPinObj, Sound);
		return true;
	}

	return false;
}

UAtomSoundBase* SAtomSoundGraphPin::GetValue() const
{
	return Cast<UAtomSoundBase>(GraphPinObj->DefaultObject);
}

#undef LOCTEXT_NAMESPACE
