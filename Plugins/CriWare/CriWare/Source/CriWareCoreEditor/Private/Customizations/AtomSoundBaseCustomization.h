/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : AtomSoundBaseCustomization.h
 *
 ****************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Layout/Visibility.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"

#include "Atom/Atom.h"
#include "Atom/AtomSoundBase.h"
#include "Atom/AtomSoundBank.h"
#include "Atom/AtomWaveBank.h"
#include "Atom/AtomCueSheet.h"

#include "AtomSoundBaseCustomization.generated.h"

class SAssetView;

/** Base class to hold a SoundBank property and permit SoundBank type selection. (Internal Use only) */
UCLASS(Transient)
class UAtomSoundBankPropertyHolder_Base
	: public UObject
{
	GENERATED_BODY()

public:

	virtual void SetSoundAsset(TScriptInterface<IAtomSoundAsset>) {}
	virtual UAtomSoundBase* GetSoundBase() { return nullptr; }
};

/** Customization for AtomSoundBase properties with any derived type support. */
class FAtomSoundBaseCustomization_Base
	: public IPropertyTypeCustomization
{
public:

	//~ Begin IPropertyTypeCustomization
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	//~ End IPropertyTypeCustomization

protected:

	virtual bool CanCustomize(TSharedRef<IPropertyHandle> StructPropertyHandle) = 0;
	virtual TStrongObjectPtr<UAtomSoundBankPropertyHolder_Base> CreateSoundBankPropertyHolder() { return nullptr; }

private:

	/** local sound bank. */
	TStrongObjectPtr<UAtomSoundBankPropertyHolder_Base> SoundBankHolder;
	TSharedPtr<IPropertyHandle> SoundBankHandle;
	TSharedPtr<ISinglePropertyView> SoundBankView;

	/** AtomSoundBase being edited. */
	TSharedPtr<IPropertyHandle> SoundBaseHandle;

	EVisibility IsSoundBankSelected() const;

	void OnSoundBankPropertyChanged();
	void OnSoundBasePropertyChanged();
	UAtomSoundBase* GenerateCachedSounds();
	bool IsSoundComboBoxEnabled() const;
	TSharedRef<SWidget> OnGenerateSoundComboBoxWidget(UAtomSoundBase* Item);
	void OnSoundComboBoxOpening();
	void OnSoundComboBoxSelectionChanged(UAtomSoundBase* Item, ESelectInfo::Type SelectInfo);
	UObject* GetSoundComboBoxContent() const;
	FText GetSoundComboBoxToolTip() const;

	// SAssetView
	bool OnIsAssetAcceptableForDropWithReason(const FAssetData& Asset, FText& OutReason);
	void OnAssetDropped(const FDragDropEvent&, const FAssetData& Asset);

	TArray<UAtomSoundBase*> CachedSoundArray;
	TSharedPtr<SComboBox<UAtomSoundBase*>> SoundComboBox;
	TSharedPtr<SAssetView> SoundComboBoxContentView;
};

/** class to hold a SoundBank property to permit any SoundBank or SoundAsset type selection. (Internal Use only) */
UCLASS(Transient)
class UAtomSoundBankPropertyHolder
	: public UAtomSoundBankPropertyHolder_Base
{
	GENERATED_BODY()

public:

	virtual void SetSoundAsset(TScriptInterface<IAtomSoundAsset> InSoundAsset) override { SoundAsset = InSoundAsset; }
	virtual UAtomSoundBase* GetSoundBase() override { return Cast<UAtomSoundBase>(SoundAsset.GetObject()); }

private:

	UPROPERTY(Transient, EditAnywhere, Category = Sound, meta = (AllowPrivateAccess))
	TScriptInterface<IAtomSoundAsset> SoundAsset = nullptr;
};

/** Customization specific to AtomSoundBase properties. */
class FAtomSoundBaseCustomization
	: public FAtomSoundBaseCustomization_Base
{
public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual bool CanCustomize(TSharedRef<IPropertyHandle> StructPropertyHandle) override;
	virtual TStrongObjectPtr<UAtomSoundBankPropertyHolder_Base> CreateSoundBankPropertyHolder() override;
};

/** class to hold a WaveBank property to permit a WaveBank selection. (Internal Use only) */
UCLASS(Transient)
class UAtomWaveBankPropertyHolder
	: public UAtomSoundBankPropertyHolder_Base
{
	GENERATED_BODY()

public:

	virtual void SetSoundAsset(TScriptInterface<IAtomSoundAsset> InSoundAsset) override { SoundAsset = Cast<UAtomWaveBank>(InSoundAsset.GetObject()); }

private:

	/** A WaveBank to select a sound wave from. */
	UPROPERTY(Transient, EditAnywhere, Category = Sound, meta = (AllowPrivateAccess))
	TObjectPtr<UAtomWaveBank> SoundAsset;
};

/** Customization specific to AtomSoundWave properties. */
class FAtomSoundWaveCustomization
	: public FAtomSoundBaseCustomization_Base
{
public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual bool CanCustomize(TSharedRef<IPropertyHandle> StructPropertyHandle) override { return true; }
	virtual TStrongObjectPtr<UAtomSoundBankPropertyHolder_Base> CreateSoundBankPropertyHolder() override;
};

/** class to hold a CueSheet property to permit a CueSheet selection. (Internal Use only) */
UCLASS(Transient)
class UAtomCueSheetPropertyHolder
	: public UAtomSoundBankPropertyHolder_Base
{
	GENERATED_BODY()

public:

	virtual void SetSoundAsset(TScriptInterface<IAtomSoundAsset> InSoundAsset) override { SoundAsset = Cast<UAtomCueSheet>(InSoundAsset.GetObject()); }

private:

	UPROPERTY(Transient, EditAnywhere, Category = Sound, meta = (AllowPrivateAccess))
	TObjectPtr<UAtomCueSheet> SoundAsset;
};

/** Customization specific to AtomSoundCue properties. */
class FAtomSoundCueCustomization
	: public FAtomSoundBaseCustomization
{
public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual bool CanCustomize(TSharedRef<IPropertyHandle> StructPropertyHandle) override { return true; }
	virtual TStrongObjectPtr<UAtomSoundBankPropertyHolder_Base> CreateSoundBankPropertyHolder() override;
};
