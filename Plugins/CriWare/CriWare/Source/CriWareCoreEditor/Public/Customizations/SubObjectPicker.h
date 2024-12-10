/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : SubObjectPicker.h
 *
 ****************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Templates/SharedPointer.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Framework/SlateDelegates.h"
#include "IDetailCustomization.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyHandle.h"
#include "Delegates/Delegate.h"

#include "Atom/AtomConfig.h"
#include "Atom/AtomRack.h"
#include "Atom/AtomSoundBank.h"
#include "Atom/AtomSoundBase.h"

#include "SubObjectPicker.generated.h"

// Forward Declarations
class SAssetView;

/** Called to add extra asset data to the asset view, to display virtual assets. These get treated similar to Class assets */
DECLARE_DELEGATE_TwoParams(FOnGetSubObjectAssets, TSharedPtr<IPropertyHandle> /*ParentHandle*/, TArray<FAssetData>& /*SubObjectAssets*/)

struct FSubObjectPickerConfig
{
	TSharedPtr<IPropertyHandle> ObjectHandle = nullptr;

	/** parent holder property */
	TSharedPtr<IPropertyHandle> ParentHandle = nullptr;

	FOnGetSubObjectAssets OnGetSubObjects;

	bool bDisplayThumbnail = true;

	float ThumbnailSize = 48.0f;

	bool bOpenAssetOnDoubleClick = false;

	bool bAllowAssetDragging = false;
	bool bAllowAssetDropping = false;
};

class CRIWARECOREEDITOR_API FSubObjectPicker
	: public TSharedFromThis<FSubObjectPicker>
{
public:

	FSubObjectPicker(const FSubObjectPickerConfig& Config);
	~FSubObjectPicker();

	TSharedRef<SWidget> CreateParentObjectPropertyNameWidget();
	TSharedRef<SWidget> CreateParentObjectPropertyValueWidget();
	TSharedRef<SWidget> CreateSubObjectPropertyNameWidget();
	TSharedRef<SWidget> CreateSubObjectPropertyValueWidget();

	void AddParentPropertyRowContent(FDetailWidgetRow& WidgetRow);
	void AddSubObjectPropertyRowContent(FDetailWidgetRow& WidgetRow, bool bIsAlwaysVisible = false);

private:

	/** property being edited. */
	TSharedPtr<IPropertyHandle> ObjectHandle;

	/** parent holder property */
	TSharedPtr<IPropertyHandle> ParentHandle;

	FOnGetSubObjectAssets OnGetSubObjects;

	bool bDisplayThumbnail;

	float ThumbnailSize;

	bool bOpenAssetOnDoubleClick;

	bool bAllowAssetDragging;
	bool bAllowAssetDropping;

	EVisibility IsParentSelected() const;

	void OnParentPropertyChanged();
	void OnObjectPropertyChanged();
	UObject* GenerateCachedObjects();
	bool IsObjectComboBoxEnabled() const;
	TSharedRef<SWidget> OnGenerateObjectComboBoxWidget(UObject* Item);
	void OnObjectComboBoxOpening();
	void OnObjectComboBoxSelectionChanged(UObject* Item, ESelectInfo::Type SelectInfo);
	UObject* GetObjectComboBoxContent() const;
	FText GetObjectComboBoxToolTip() const;

	void GenerateArrayWidget(TSharedRef<IPropertyHandle> PropertyHandle, int32 ArrayIndex, IDetailChildrenBuilder& ChildrenBuilder);
	void OnAssetSelected(const FAssetData& AssetData);
	void OnCloseComboButton();

	TArray<UObject*> CachedObjectArray;
	TSharedPtr<SComboBox<UObject*>> ObjectComboBox;
	TSharedPtr<SAssetView> ObjectComboBoxContentView;
};

/** UClass to hold the parent property and permit subobject selection. (Need to derive.) */
UCLASS(Transient)
class CRIWARECOREEDITOR_API UParentObjectPropertyHolder_Base
	: public UObject
{
	GENERATED_BODY()

public:

	TSharedPtr<IPropertyHandle> GetProperty() const { return PropertyHandle; }

	// Interface to access to children objects
	virtual int GetNumSubObjects() { return 0; }
	virtual UObject* GetSubObject(int Index) { return nullptr; }

protected:

	TSharedPtr<IPropertyHandle> PropertyHandle;

	/** Lets generate the property handle from a temporary property in a derived class. */
	TSharedPtr<IPropertyHandle> MakeHandleFromProperty(FName PropertyName);

private:

	TSharedPtr<ISinglePropertyView> ParentView;
};

/** Class to hold a AtomConfig property and permit config subobject selection (patches, settings ...). (Internal Use only) */
UCLASS(Transient, MinimalAPI)
class UAtomConfigPropertyHolder_Internal
	: public UParentObjectPropertyHolder_Base
{
	GENERATED_BODY()

public:

	enum Target
	{
		GlobalAisacs,
		MixerSettings,
	};

	void Init(Target InTarget = Target::GlobalAisacs)
	{ 
		Target = InTarget;
		MakeHandleFromProperty(TEXT("Config"));
	}

	virtual int GetNumSubObjects() override
	{
		switch (Target)
		{
		case Target::GlobalAisacs: return Config ? Config->GetNumGlobalAisacs() : 0;
		case Target::MixerSettings: return Config ? Config->GetNumDspBusSettings() : 0;
		default: return 0;
		}
	}

	virtual UObject* GetSubObject(int Index) override
	{
		switch (Target)
		{
		case Target::GlobalAisacs: return Config ? Config->GetGlobalAisac(Index) : nullptr;
		case Target::MixerSettings: return Config ? Config->GetDspBusSetting(Index) : nullptr;
		default: return nullptr;
		}
	}

private:

	UPROPERTY(Transient, EditAnywhere, Category = Config, meta = (AllowPrivateAccess))
	TObjectPtr<UAtomConfig> Config = nullptr;

	Target Target = GlobalAisacs;
};

/** Class to hold a Rack property and permit inner bus selection. (Internal Use only) */
UCLASS(Transient, MinimalAPI)
class UAtomRackPropertyHolder_Internal
	: public UParentObjectPropertyHolder_Base
{
	GENERATED_BODY()

public:

	void Init() { MakeHandleFromProperty(TEXT("Rack")); }

	virtual int GetNumSubObjects() { return !Rack ? 0 : Cast<UAtomRack>(Rack) ? ((UAtomRack*)Rack)->Buses.Num() : 1; }
	virtual UObject* GetSubObject(int Index) { return !Rack ? nullptr : Cast<UAtomRack>(Rack) && ((UAtomRack*)Rack)->Buses.IsValidIndex(Index) ? ((UAtomRack*)Rack)->Buses[Index] : Rack->MasterBus; }

private:

	UPROPERTY(Transient, EditAnywhere, Category = Bus, meta = (AllowPrivateAccess))
	TObjectPtr<UAtomRackWithParentBase> Rack = nullptr;
};

/** Class to hold a AtomSoundBase property and inner sound selection (Cue, Waves ...). (Internal Use only) */
UCLASS(Transient, MinimalAPI)
class UAtomSoundBankPropertyHolder_Internal
	: public UParentObjectPropertyHolder_Base
{
	GENERATED_BODY()

public:

	void Init() { MakeHandleFromProperty(TEXT("Sound Bank")); }

	virtual int GetNumSubObjects() override { return Bank ? Bank->GetNumSounds() : 0; }
	virtual UObject* GetSubObject(int Index) override { return Bank ? Bank->GetSound(Index) : nullptr; }

private:

	UPROPERTY(Transient, EditAnywhere, Category = Config, meta = (AllowPrivateAccess))
	TObjectPtr<UAtomSoundBank> Bank = nullptr;
};
