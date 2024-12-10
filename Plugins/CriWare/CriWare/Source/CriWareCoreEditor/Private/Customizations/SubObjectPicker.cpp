/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : SubObjectPicker.cpp
 *
 ****************************************************************************/

#include "Customizations/SubObjectPicker.h"

#include "SlateGlobals.h"
#include "Widgets/Text/STextBlock.h"
#include "PropertyRestriction.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailPropertyRow.h"
#include "ISinglePropertyView.h"
#include "PropertyCustomizationHelpers.h"

#include "Widgets/SAssetView.h"

#define LOCTEXT_NAMESPACE "FSubObjectPicker"

namespace FSubObjectPickerUtils
{
	bool IsPropertyValueChildOf(TSharedPtr<IPropertyHandle> InProp, const UStruct* Class)
	{
		if (InProp.IsValid() && InProp->IsValidHandle())
		{
			UObject* Val;
			InProp->GetValue(Val);
			return (Val && Val->GetClass()->IsChildOf(Class));
		}
		return false;
	}
}

TSharedPtr<IPropertyHandle> UParentObjectPropertyHolder_Base::MakeHandleFromProperty(FName PropertyName)
{
	// create a property widget from PropertyEditor module
	const FSinglePropertyParams InitParams;
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	ParentView = PropertyEditorModule.CreateSingleProperty(this, PropertyName, InitParams);

	if (ParentView)
	{
		PropertyHandle = ParentView->GetPropertyHandle();
	}

	return PropertyHandle;
}

FSubObjectPicker::FSubObjectPicker(const FSubObjectPickerConfig& Config)
	: ObjectHandle(Config.ObjectHandle)
	, ParentHandle(Config.ParentHandle)
	, OnGetSubObjects(Config.OnGetSubObjects)
	, bDisplayThumbnail(Config.bDisplayThumbnail)
	, ThumbnailSize(Config.ThumbnailSize)
	, bOpenAssetOnDoubleClick(Config.bOpenAssetOnDoubleClick)
	, bAllowAssetDragging(Config.bAllowAssetDragging)
	, bAllowAssetDropping(Config.bAllowAssetDropping)
{
}

FSubObjectPicker::~FSubObjectPicker()
{}

TSharedRef<SWidget> FSubObjectPicker::CreateParentObjectPropertyNameWidget()
{
	if (ParentHandle.IsValid() && ParentHandle->IsValidHandle())
	{
		TSharedRef<SWidget> Widget = ParentHandle->CreatePropertyNameWidget();
		Widget->SetEnabled(TAttribute<bool>::Create([Handle = ObjectHandle]()
		{
			return Handle.IsValid() ? Handle->IsEditable() : true;
		}));
		return Widget;
	}

	return SNullWidget::NullWidget;
}

TSharedRef<SWidget> FSubObjectPicker::CreateParentObjectPropertyValueWidget()
{
	if (ParentHandle.IsValid() && ParentHandle->IsValidHandle())
	{
		TSharedRef<SWidget> Widget = ParentHandle->CreatePropertyValueWidget();
		Widget->SetEnabled(TAttribute<bool>::Create([Handle = ObjectHandle]()
		{
			return Handle.IsValid() ? Handle->IsEditable() : true;
		}));
		return Widget;
	}

	return SNullWidget::NullWidget;
}

void FSubObjectPicker::AddParentPropertyRowContent(FDetailWidgetRow& WidgetRow)
{
	WidgetRow.NameContent()
	[
		CreateParentObjectPropertyNameWidget()
	]
	.ValueContent()
	[
		CreateParentObjectPropertyValueWidget()
	];
}

TSharedRef<SWidget> FSubObjectPicker::CreateSubObjectPropertyNameWidget()
{
	return SNew(STextBlock)
		.Text_Lambda([this]()
			{
				FText PropertyName = ObjectHandle.IsValid() ? ObjectHandle->GetPropertyDisplayName() : LOCTEXT("SubObjectPickerLabel", "Object");
				return	PropertyName;
			})
		.ToolTipText_Lambda([this]()
			{
				return	LOCTEXT("SubObjectPickerTooltip", "The object to use.");
			})
		.Font(IDetailLayoutBuilder::GetDetailFont());
}

TSharedRef<SWidget> FSubObjectPicker::CreateSubObjectPropertyValueWidget()
{
	if (!ObjectHandle->IsValidHandle())
	{
		return SNullWidget::NullWidget;
	}

	ObjectHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FSubObjectPicker::OnObjectPropertyChanged));
	UObject* Object = nullptr;
	ObjectHandle->GetValue(Object);

	if (ParentHandle.IsValid() && ParentHandle->IsValidHandle())
	{
		ParentHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FSubObjectPicker::OnParentPropertyChanged));
	}

	// -- Setup a ComboBox to select a sub object from an object
	TObjectPtr<UObject> InitialSelectedSetting = GenerateCachedObjects();

	// TODO: REPLACE combobox by assetviwer like now with conetentbrowser's picker drop menu -> SAssetPicker
	return SAssignNew(ObjectComboBox, SComboBox<UObject*>)
		.OptionsSource(&CachedObjectArray)
		.IsEnabled(this, &FSubObjectPicker::IsObjectComboBoxEnabled)
		.OnGenerateWidget(this, &FSubObjectPicker::OnGenerateObjectComboBoxWidget)
		.OnComboBoxOpening(this, &FSubObjectPicker::OnObjectComboBoxOpening)
		.OnSelectionChanged(this, &FSubObjectPicker::OnObjectComboBoxSelectionChanged)
		.InitiallySelectedItem(InitialSelectedSetting)
		.ContentPadding(FMargin(2.0f, 2.0f))
		.Content()
		[
			SAssignNew(ObjectComboBoxContentView, SAssetView)
			.Object(Cast<UObject>(Object))
			.DisplayThumbnail(bDisplayThumbnail)
			.ThumbnailSize(ThumbnailSize)
			.OpenAssetOnDoubleClick(bOpenAssetOnDoubleClick)
			.AllowAssetDragging(bAllowAssetDragging)
			.AllowAssetDropping(bAllowAssetDropping)
			.OnIsAssetAcceptableForDropWithReason_Lambda([&](const FAssetData& Asset, FText& OutReason)
			{
				if (ObjectHandle.IsValid() && ObjectHandle->IsValidHandle())
				{
					auto ObjectProperty = CastField<FObjectProperty>(ObjectHandle->GetProperty());
					return ObjectProperty && ObjectProperty->PropertyClass == Asset.GetClass();
				}
				return false;
			})
			.OnAssetDropped_Lambda([&](const FDragDropEvent&, const FAssetData& Asset)
			{
				if (ObjectHandle.IsValid() && ObjectHandle->IsValidHandle())
				{
					if (auto Object = Asset.GetAsset())
					{
						ObjectHandle->SetValue(Object);
						ObjectComboBoxContentView->SetObject(Object);
					}
				}
			})
			//.ToolTipText(this, &FAtomBusCustomization::GetCueComboBoxToolTip)
		];
}

void FSubObjectPicker::AddSubObjectPropertyRowContent(FDetailWidgetRow& WidgetRow, bool bIsAlwaysVisible /* = false */)
{
	WidgetRow.NameContent()
	[
		CreateSubObjectPropertyNameWidget()
	]
	.ValueContent()
	[
		CreateSubObjectPropertyValueWidget()
	];

	if (!bIsAlwaysVisible)
	{
		WidgetRow.Visibility(TAttribute<EVisibility>(this, &FSubObjectPicker::IsParentSelected));
	}
}

void FSubObjectPicker::GenerateArrayWidget(TSharedRef<IPropertyHandle> PropertyHandle, int32 ArrayIndex, IDetailChildrenBuilder& ChildrenBuilder)
{
	UObject* ParentObject = nullptr;
	ParentHandle->GetValue(ParentObject);

	UObject* PropertyObject = nullptr;
	PropertyHandle->GetValue(PropertyObject);

	auto ObjectProperty = CastField<FObjectProperty>(PropertyHandle->GetProperty());

	TArray<const UClass*> AllowedClasses;
	AllowedClasses.Add(ObjectProperty->PropertyClass);
	
	TArray<UFactory*> NewAssetFactories;
	TArray<FAssetData> SubObjectAssets;
	OnGetSubObjects.ExecuteIfBound(ParentHandle, SubObjectAssets);

	PropertyCustomizationHelpers::MakeAssetPickerWithMenu(
		FAssetData(PropertyObject),
		true,
		true,
		AllowedClasses,
		NewAssetFactories,
		FOnShouldFilterAsset(),
		FOnAssetSelected::CreateRaw(this, &FSubObjectPicker::OnAssetSelected),
		FSimpleDelegate::CreateSP(this, &FSubObjectPicker::OnCloseComboButton),
		PropertyHandle,
		SubObjectAssets);
}

void FSubObjectPicker::OnAssetSelected(const FAssetData& AssetData)
{

}

void FSubObjectPicker::OnCloseComboButton()
{

}

EVisibility FSubObjectPicker::IsParentSelected() const
{
	if (!FSubObjectPickerUtils::IsPropertyValueChildOf(ParentHandle, UObject::StaticClass()))
	{
		return EVisibility::Hidden;
	}

	return EVisibility::Visible;
}

void FSubObjectPicker::OnParentPropertyChanged()
{
	if (ParentHandle.IsValid() && ParentHandle->IsValidHandle())
	{
		if (ObjectComboBox)
		{
			TObjectPtr<UObject> SelectedObject = GenerateCachedObjects();
			ObjectComboBox->SetSelectedItem(SelectedObject);
			ObjectComboBox->RefreshOptions();

			// update the combobox content with selection
			if (ObjectComboBoxContentView)
			{
				ObjectComboBoxContentView->SetObject(SelectedObject);
			}

			// set also to the real property
			ObjectHandle->SetValue(SelectedObject);
		}
	}
}

void FSubObjectPicker::OnObjectPropertyChanged()
{

}

UObject* FSubObjectPicker::GenerateCachedObjects()
{
	TObjectPtr<UObject> InitialSelectedObject = nullptr;
	CachedObjectArray.Reset();
	CachedObjectArray.Add(InitialSelectedObject);

	UObject* Object = nullptr;
	ObjectHandle->GetValue(Object);

	TArray<FAssetData> SubObjectAssets;
	OnGetSubObjects.ExecuteIfBound(ParentHandle, SubObjectAssets);

	for (FAssetData& AssetData : SubObjectAssets)
	{
		UObject* SubObject = AssetData.GetAsset(); // load if not loaded!

		CachedObjectArray.Add(SubObject);

		if (SubObject && SubObject == Object)
		{
			InitialSelectedObject = Object;
		}
	}

	return InitialSelectedObject;
}

bool FSubObjectPicker::IsObjectComboBoxEnabled() const
{
	UObject* Object = nullptr;
	if (ParentHandle)
	{
		ParentHandle->GetValue(Object);
	}
	return Object != nullptr;
}

TSharedRef<SWidget> FSubObjectPicker::OnGenerateObjectComboBoxWidget(UObject* Item)
{
	return SNew(SAssetView)
		.Object(Item);
}

void FSubObjectPicker::OnObjectComboBoxOpening()
{
	TObjectPtr<UObject> SelectedObject = GenerateCachedObjects();
	ObjectComboBox->SetSelectedItem(SelectedObject);
	ObjectComboBox->RefreshOptions();
}

void FSubObjectPicker::OnObjectComboBoxSelectionChanged(UObject* Item, ESelectInfo::Type SelectInfo)
{
	if (Item)
	{
		// update the combobox content with selection
		ObjectComboBoxContentView->SetObject(Item);

		// set the selection to property
		ObjectHandle->SetValue(Item);
	}
}

UObject* FSubObjectPicker::GetObjectComboBoxContent() const
{
	if (ObjectComboBox)
	{
		return ObjectComboBox->GetSelectedItem();
	}

	return nullptr;
}

FText FSubObjectPicker::GetObjectComboBoxToolTip() const
{
	if (ObjectComboBox)
	{
		FName Name = ObjectComboBox->GetSelectedItem()->GetFName();
		return FText::FromName(Name);
	}

	return FText::FromName(NAME_None);
}

#undef LOCTEXT_NAMESPACE
