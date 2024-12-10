/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomBusDetails.cpp
 *
 ****************************************************************************/

#include "AtomBusDetails.h"

#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"

#include "Atom/AtomBus.h"
#include "Atom/AtomBusEffectPreset.h"

#define LOCTEXT_NAMESPACE	"AtomBusDetailsInspector"

TSharedRef<IDetailCustomization> FAtomBusDetails::MakeInstance()
{
	return MakeShareable(new FAtomBusDetails);
}

void FAtomBusDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	// Get Effects category
	IDetailCategoryBuilder& CategoryBuilder = DetailLayout.EditCategory(TEXT("Effects"));

	// Hide effect classes
	TSharedPtr<IPropertyHandle> BusEffectClassesHandle = DetailLayout.GetProperty(TEXT("BusEffectChainClasses"));
	BusEffectClassesHandle->MarkHiddenByCustomization();

	// cutomize effect chain array
	TSharedPtr<IPropertyHandle> BusEffectChainHandle = DetailLayout.GetProperty(TEXT("BusEffectChain"));
	TSharedRef<FDetailArrayBuilder> BusEffectChainBuilder = MakeShareable(new FDetailArrayBuilder(BusEffectChainHandle.ToSharedRef(), false));
	BusEffectChainBuilder->OnGenerateArrayElementWidget(FOnGenerateArrayElementWidget::CreateSP(this, &FAtomBusDetails::GenerateBusEffectPresetWidget));
	CategoryBuilder.AddCustomBuilder(BusEffectChainBuilder);
}

void FAtomBusDetails::GenerateBusEffectPresetWidget(TSharedRef<IPropertyHandle> PropertyHandle, int32 ArrayIndex, IDetailChildrenBuilder& ChildrenBuilder)
{
	IDetailPropertyRow& EffectPresetRow = ChildrenBuilder.AddProperty(PropertyHandle);

	FNumberFormattingOptions NoCommas;
	NoCommas.UseGrouping = false;
	UObject* BusEffectPresetObject = nullptr;
	PropertyHandle->GetValue(BusEffectPresetObject);
	if (UAtomBusEffectPreset* EffectPreset = Cast<UAtomBusEffectPreset>(BusEffectPresetObject))
	{
		const FText SlotDesc = FText::Format(LOCTEXT("BusEffectChainName", "Effect #{0} - {1}"), FText::AsNumber(ArrayIndex, &NoCommas), FText::FromName(EffectPreset->GetEffectName()));
		EffectPresetRow.DisplayName(SlotDesc);
	}
	else
	{
		const FText SlotDesc = FText::Format(LOCTEXT("BusEffectChainIndex", "Effect #{0}"), FText::AsNumber(ArrayIndex, &NoCommas));
		EffectPresetRow.DisplayName(SlotDesc);
	}

	EffectPresetRow.ShowPropertyButtons(false);

	FUIAction CopyAction;
	FUIAction PasteAction;
	PropertyHandle->CreateDefaultPropertyCopyPasteActions(CopyAction, PasteAction);

	EffectPresetRow.CustomWidget(false)
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MaxDesiredWidth(TOptional<float>())
		[
			PropertyHandle->CreatePropertyValueWidget()
		]
		.CopyAction(CopyAction)
		.PasteAction(PasteAction);
}

#undef LOCTEXT_NAMESPACE
