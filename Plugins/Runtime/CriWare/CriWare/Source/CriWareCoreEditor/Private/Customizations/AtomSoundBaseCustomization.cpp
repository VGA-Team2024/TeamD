/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomSoundBaseCustomization.cpp
 *
 ****************************************************************************/

#include "AtomSoundBaseCustomization.h"

#include "Widgets/Text/STextBlock.h"
#include "PropertyRestriction.h"
#include "IDetailChildrenBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailPropertyRow.h"
#include "AssetRegistry/AssetData.h"
#include "AssetThumbnail.h"
#include "SAssetDropTarget.h"

#include "Atom/Atom.h"
#include "Atom/AtomSoundCue.h"
#include "Atom/AtomSoundWave.h"

#include "Widgets/SAssetView.h"

#define LOCTEXT_NAMESPACE "AtomSoundBase"

namespace AtomSoundBaseUtils
{
	bool IsPropertyValueChildOf(TSharedPtr<IPropertyHandle> InProp, const UStruct* Class)
	{
		if (InProp.IsValid())
		{
			UObject* Val;
			InProp->GetValue(Val);
			return (Val && Val->GetClass()->IsChildOf(Class));
		}
		return false;
	}

	bool IsClassChildOfPropertyClass(const UStruct* Class, TSharedPtr<IPropertyHandle> InProp)
	{
		if (InProp.IsValid())
		{
			//~ CRI
			auto Prop = CastField<FObjectProperty>(InProp->GetProperty());
			if (Prop)
			{
				return (Prop && Class->IsChildOf(Prop->PropertyClass));
			}

			auto SoftProp = CastField<FSoftObjectProperty>(InProp->GetProperty());
			if (SoftProp)
			{
				return (SoftProp && Class->IsChildOf(SoftProp->PropertyClass));
			}

			// in case it's the UAtomSoundAsset interface we use the UAtomSoundBank as wild card.
			auto InterProp = CastField<FInterfaceProperty>(InProp->GetProperty());
			if (Class->GetOwnerClass()->ImplementsInterface(InterProp->InterfaceClass))
			{
				return (InterProp && Class->IsChildOf(UAtomSoundBank::StaticClass()));
			}
			//~
		}
		return false;
	}

	//~ CRI
	UObject* GetValueAsObject(TSharedPtr<IPropertyHandle> PropertyHandle)
	{
		UObject* Object = nullptr;
		PropertyHandle->GetValue(Object);

		if (Object == nullptr)
		{
			// Check to see if it's pointing to an unloaded object
			FString CurrentObjectPath;
			PropertyHandle->GetValueAsFormattedString(CurrentObjectPath);

			if (CurrentObjectPath.Len() > 0 && CurrentObjectPath != TEXT("None"))
			{
				FSoftObjectPath SoftObjectPath = FSoftObjectPath(CurrentObjectPath);
				Object = SoftObjectPath.TryLoad();
			}
		}

		return Object;
	}
	//~
}

/* FAtomSoundBaseCustomization_Base class
 *****************************************************************************/

void FAtomSoundBaseCustomization_Base::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	// Property handle here is the base struct. We are going to hide it since we're showing it's properties directly.
	StructPropertyHandle->MarkHiddenByCustomization();
}

void FAtomSoundBaseCustomization_Base::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	// Sound type without bank are not customized.
	if (!CanCustomize(StructPropertyHandle))
	{
		ChildBuilder.AddCustomRow(FText::GetEmpty())
			.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			]
			.ValueContent()
			[
				StructPropertyHandle->CreatePropertyValueWidget()
			];
		return;
	}

	// Get handle to layout builder to enable adding properties to categories
	//IDetailLayoutBuilder& LayoutBuilder = ChildBuilder.GetParentCategory().GetParentLayout();

	SoundBaseHandle = StructPropertyHandle;
	SoundBaseHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FAtomSoundBaseCustomization_Base::OnSoundBasePropertyChanged));

	// Create a fake sound property to show sound bank and select property from it.
	SoundBankHolder = CreateSoundBankPropertyHolder();

	// Set default value
	//~ CRI
	UObject* SoundBaseObject = AtomSoundBaseUtils::GetValueAsObject(SoundBaseHandle);
	//~

	if (UAtomSoundCue* Cue = Cast<UAtomSoundCue>(SoundBaseObject))
	{
		SoundBankHolder.Get()->SetSoundAsset(Cue->CueSheet);
	}
	else if (UAtomSoundWave* Wave = Cast<UAtomSoundWave>(SoundBaseObject))
	{
		SoundBankHolder.Get()->SetSoundAsset(Wave->WaveBank);
	}
	else if (UAtomSoundBase* Sound = Cast<UAtomSoundBase>(SoundBaseObject))
	{
		SoundBankHolder.Get()->SetSoundAsset(Sound);
	}
	else
	{
		SoundBankHolder.Get()->SetSoundAsset(nullptr);
	}

	// Add a property row from SoundBankHolder object
	IDetailPropertyRow* PropertyRow = ChildBuilder.AddExternalObjectProperty({ SoundBankHolder.Get() }, FName(TEXT("SoundAsset")));
	if (PropertyRow)
	{
		SoundBankHandle = PropertyRow->GetPropertyHandle();
		SoundBankHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FAtomSoundBaseCustomization_Base::OnSoundBankPropertyChanged));
		PropertyRow->CustomWidget()
			.NameContent()
			[
				SoundBankHandle->CreatePropertyNameWidget()
			]
			.ValueContent()
			[
				SNew(SAssetDropTarget)
				.OnAreAssetsAcceptableForDropWithReason_Lambda([&](TArrayView<FAssetData> InAssets, FText& OutReason) -> bool
				{
					if (InAssets.Num() > 0)
					{
						return AtomSoundBaseUtils::IsClassChildOfPropertyClass(InAssets[0].GetClass(), SoundBankHandle) 
							|| OnIsAssetAcceptableForDropWithReason(InAssets[0], OutReason);
					}
					return false;
				})
				.OnAssetsDropped_Lambda([&](const FDragDropEvent& Event, TArrayView<FAssetData> InAssets) -> void
				{
					if (InAssets.Num() > 0)
					{
						if (AtomSoundBaseUtils::IsClassChildOfPropertyClass(InAssets[0].GetClass(), SoundBankHandle))
						{
							SoundBankHolder.Get()->SetSoundAsset(InAssets[0].GetAsset());
							SoundBaseHandle->SetValue((UObject*)nullptr);
							SoundComboBoxContentView->SetObject(nullptr);
						}
						else
						{
							OnAssetDropped(Event, InAssets[0]);
						}
					}
				})
				.Content()
				[
					SoundBankHandle->CreatePropertyValueWidget()
				]
			];

		//ChildBuilder.AddProperty(SoundBankHandle.ToSharedRef())
		//	.DisplayName(SoundBaseHandle.IsValid() ? SoundBaseHandle->GetPropertyDisplayName() : LOCTEXT("AtomSoundBaseDetailLabel", "Sound"));
	}

	// -- Setup a ComboBox to select a Sonudbase from a bank
	TObjectPtr<UAtomSoundBase> InitialSelectedSetting = GenerateCachedSounds();

	ChildBuilder.AddCustomRow(FText::GetEmpty())
		.NameContent()
		[
			SNew(STextBlock)
			.Text_Lambda([this]() { FText PropertyName = SoundBaseHandle.IsValid() ? SoundBaseHandle->GetPropertyDisplayName() : LOCTEXT("AtomSoundBaseDetailLabel", "Sound");
				return	AtomSoundBaseUtils::IsPropertyValueChildOf(SoundBankHandle, UAtomCueSheet::StaticClass()) ? FText::Format(LOCTEXT("AtomSoundBaseDetailCueLabel", "{0} (Cue)"), PropertyName) :
						AtomSoundBaseUtils::IsPropertyValueChildOf(SoundBankHandle, UAtomWaveBank::StaticClass()) ? FText::Format(LOCTEXT("AtomSoundBaseDetailWaveLabel", "{0} (Wave)"), PropertyName) :
						FText::Format(LOCTEXT("AtomSoundBaseDetailLabel", "{0} (Sound)"), PropertyName);
			})
			.ToolTipText_Lambda([this]() {
				return	AtomSoundBaseUtils::IsPropertyValueChildOf(SoundBankHandle, UAtomCueSheet::StaticClass()) ? LOCTEXT("AtomSoundBaseDetailCueTooltip", "The cue in the AtomCueSheet to use.") :
						AtomSoundBaseUtils::IsPropertyValueChildOf(SoundBankHandle, UAtomWaveBank::StaticClass()) ? LOCTEXT("AtomSoundBaseDetailWaveTooltip", "The wave in the AtomWaveBank to use.") :
						LOCTEXT("AtomSoundBaseDetailTooltip", "The sound to use.");
			})
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SAssignNew(SoundComboBox, SComboBox<UAtomSoundBase*>)
			.OptionsSource(&CachedSoundArray)
			.IsEnabled(this, &FAtomSoundBaseCustomization_Base::IsSoundComboBoxEnabled)
			.OnGenerateWidget(this, &FAtomSoundBaseCustomization_Base::OnGenerateSoundComboBoxWidget)
			.OnComboBoxOpening(this, &FAtomSoundBaseCustomization_Base::OnSoundComboBoxOpening)
			.OnSelectionChanged(this, &FAtomSoundBaseCustomization_Base::OnSoundComboBoxSelectionChanged)
			.InitiallySelectedItem(InitialSelectedSetting)
			.ContentPadding(FMargin(2.0f, 2.0f))
			.Content()
			[
				SAssignNew(SoundComboBoxContentView, SAssetView)
				.Object(Cast<UAtomSoundBase>(SoundBaseObject))
				.AllowAssetDropping(true)
				.OnIsAssetAcceptableForDropWithReason(this, &FAtomSoundBaseCustomization_Base::OnIsAssetAcceptableForDropWithReason)
				.OnAssetDropped(this, &FAtomSoundBaseCustomization_Base::OnAssetDropped)
				//.ToolTipText(this, &FAtomSoundBaseCustomization_Base::GetCueComboBoxToolTip)
			]
		]
		.Visibility(TAttribute<EVisibility>(this, &FAtomSoundBaseCustomization_Base::IsSoundBankSelected));
}

bool FAtomSoundBaseCustomization_Base::OnIsAssetAcceptableForDropWithReason(const FAssetData& Asset, FText& OutReason)
{
	if (SoundBaseHandle.IsValid() && SoundBaseHandle->IsValidHandle())
	{
		return AtomSoundBaseUtils::IsClassChildOfPropertyClass(Asset.GetClass(), SoundBaseHandle);
	}
	return false;
}

void FAtomSoundBaseCustomization_Base::OnAssetDropped(const FDragDropEvent&, const FAssetData& Asset)
{
	if (SoundBaseHandle.IsValid() && SoundBaseHandle->IsValidHandle())
	{
		if (auto Object = Asset.GetAsset())
		{
			// Set bank default value
			if (UAtomSoundCue* Cue = Cast<UAtomSoundCue>(Object))
			{
				SoundBankHolder.Get()->SetSoundAsset(Cue->CueSheet);
			}
			else if (UAtomSoundWave* Wave = Cast<UAtomSoundWave>(Object))
			{
				SoundBankHolder.Get()->SetSoundAsset(Wave->WaveBank);
			}
			else if (UAtomSoundBase* Sound = Cast<UAtomSoundBase>(Object))
			{
				SoundBankHolder.Get()->SetSoundAsset(Sound);
			}
			else
			{
				SoundBankHolder.Get()->SetSoundAsset(nullptr);
			}

			SoundBaseHandle->SetValue(Object);
			SoundComboBoxContentView->SetObject(Object);
		}
	}
}

EVisibility FAtomSoundBaseCustomization_Base::IsSoundBankSelected() const
{
	if (!AtomSoundBaseUtils::IsPropertyValueChildOf(SoundBankHandle, UAtomSoundBank::StaticClass()))
	{
		return EVisibility::Hidden;
	}

	return EVisibility::Visible;
}

void FAtomSoundBaseCustomization_Base::OnSoundBankPropertyChanged()
{
	if (auto Holder = SoundBankHolder.Get())
	{
		if (UAtomSoundBase* SoundBase = Cast<UAtomSoundBase>(Holder->GetSoundBase()))
		{
			SoundBaseHandle->SetValue(SoundBase);
			return;
		}

		if (SoundComboBox)
		{
			TObjectPtr<UAtomSoundBase> SelectedSound = GenerateCachedSounds();
			SoundComboBox->SetSelectedItem(SelectedSound);
			SoundComboBox->RefreshOptions();

			// update the combobox content with selection
			if (SoundComboBoxContentView)
			{
				SoundComboBoxContentView->SetObject(SelectedSound);
			}

			// set also to the real property
			SoundBaseHandle->SetValue(SelectedSound);
		}
	}
}

void FAtomSoundBaseCustomization_Base::OnSoundBasePropertyChanged()
{

}

UAtomSoundBase* FAtomSoundBaseCustomization_Base::GenerateCachedSounds()
{
	TObjectPtr<UAtomSoundBase> InitialSelectedSound = nullptr;
	CachedSoundArray.Reset();
	CachedSoundArray.Add(InitialSelectedSound);

	UObject* SoundBankObject = nullptr;
	if (SoundBankHandle)
	{
		SoundBankHandle->GetValue(SoundBankObject);
	}

	UObject* SoundBaseObject = nullptr;
	SoundBaseHandle->GetValue(SoundBaseObject);

	if (UAtomCueSheet* AtomCueSheet = Cast<UAtomCueSheet>(SoundBankObject))
	{
		for (int Index = 0; AtomCueSheet && Index < AtomCueSheet->GetNumCues(); ++Index)
		{
			UAtomSoundCue* SoundCue = AtomCueSheet->GetSoundCue(Index);

			CachedSoundArray.Add(SoundCue);

			if (SoundBaseObject && SoundBaseObject == SoundCue)
			{
				InitialSelectedSound = SoundCue;
			}
		}
	}
	else if (UAtomWaveBank* AtomWaveBank = Cast<UAtomWaveBank>(SoundBankObject))
	{
		for (int Index = 0; AtomWaveBank && Index < AtomWaveBank->GetNumWaves(); ++Index)
		{
			UAtomSoundWave* SoundWave = AtomWaveBank->GetSoundWave(Index);

			CachedSoundArray.Add(SoundWave);

			if (SoundBaseObject && SoundBaseObject == SoundWave)
			{
				InitialSelectedSound = SoundWave;
			}
		}
	}

	return InitialSelectedSound;
}

bool FAtomSoundBaseCustomization_Base::IsSoundComboBoxEnabled() const
{
	UObject* AtomSoundBase = nullptr;
	if (SoundBankHandle)
	{
		SoundBankHandle->GetValue(AtomSoundBase);
	}
	return AtomSoundBase != nullptr;
}

TSharedRef<SWidget> FAtomSoundBaseCustomization_Base::OnGenerateSoundComboBoxWidget(UAtomSoundBase* Item)
{
	return SNew(SAssetView)
		.Object(Item);
}

void FAtomSoundBaseCustomization_Base::OnSoundComboBoxOpening()
{
	TObjectPtr<UAtomSoundBase> SelectedSound = GenerateCachedSounds();
	SoundComboBox->SetSelectedItem(SelectedSound);
	SoundComboBox->RefreshOptions();
}

void FAtomSoundBaseCustomization_Base::OnSoundComboBoxSelectionChanged(UAtomSoundBase* Item, ESelectInfo::Type SelectInfo)
{
	if (Item)
	{
		// update the combobox content with selection
		SoundComboBoxContentView->SetObject(Item);

		// set the selection to property
		SoundBaseHandle->SetValue(Item);
	}
}

UObject* FAtomSoundBaseCustomization_Base::GetSoundComboBoxContent() const
{
	if (SoundComboBox)
	{
		return SoundComboBox->GetSelectedItem();
	}

	return nullptr;
}

FText FAtomSoundBaseCustomization_Base::GetSoundComboBoxToolTip() const
{
	if (SoundComboBox)
	{
		FName Name = SoundComboBox->GetSelectedItem()->GetFName();
		return FText::FromName(Name);
	}

	return FText::FromName(NAME_None);
}

/* FAtomSoundBaseCustomization class
 *****************************************************************************/

TSharedRef<IPropertyTypeCustomization> FAtomSoundBaseCustomization::MakeInstance()
{
	return MakeShareable(new FAtomSoundBaseCustomization);
}

TStrongObjectPtr<UAtomSoundBankPropertyHolder_Base> FAtomSoundBaseCustomization::CreateSoundBankPropertyHolder()
{
	return TStrongObjectPtr<UAtomSoundBankPropertyHolder_Base>(NewObject<UAtomSoundBankPropertyHolder>());
}

bool FAtomSoundBaseCustomization::CanCustomize(TSharedRef<IPropertyHandle> StructPropertyHandle)
{
	// only customize for any sound asset if AtomSoundBase exactly, by default derived class doesn't customize
	auto ObjectProperty = CastField<FObjectProperty>(StructPropertyHandle->GetProperty());
	bool bIsSoundBaseClass = ObjectProperty && ObjectProperty->PropertyClass == UAtomSoundBase::StaticClass();

	 if (!ObjectProperty)
	 {
		 auto SoftObjectProperty = CastField<FSoftObjectProperty>(StructPropertyHandle->GetProperty());
		 bIsSoundBaseClass = SoftObjectProperty && SoftObjectProperty->PropertyClass == UAtomSoundBase::StaticClass();
	 }

	 return bIsSoundBaseClass;
}

/* FAtomSoundWaveCustomization class
 *****************************************************************************/

TSharedRef<IPropertyTypeCustomization> FAtomSoundWaveCustomization::MakeInstance()
{
	return MakeShareable(new FAtomSoundWaveCustomization);
}

TStrongObjectPtr<UAtomSoundBankPropertyHolder_Base> FAtomSoundWaveCustomization::CreateSoundBankPropertyHolder()
{
	return TStrongObjectPtr<UAtomSoundBankPropertyHolder_Base>(NewObject<UAtomWaveBankPropertyHolder>());
}

/* FAtomSoundCueCustomization class
 *****************************************************************************/

TSharedRef<IPropertyTypeCustomization> FAtomSoundCueCustomization::MakeInstance()
{
	return MakeShareable(new FAtomSoundCueCustomization);
}

TStrongObjectPtr<UAtomSoundBankPropertyHolder_Base> FAtomSoundCueCustomization::CreateSoundBankPropertyHolder()
{
	return TStrongObjectPtr<UAtomSoundBankPropertyHolder_Base>(NewObject<UAtomCueSheetPropertyHolder>());
}

#undef LOCTEXT_NAMESPACE
