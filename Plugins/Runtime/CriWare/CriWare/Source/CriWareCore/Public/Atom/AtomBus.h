/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCore
 * File     : AtomBus.h
 *
 ****************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Templates/SubclassOf.h"

#include "Atom.h"
#include "AtomBusEffectPreset.h" 
#include "Modulation/AtomModulationDestination.h"

#include "AtomBus.generated.h"

// Forward Defenitions
class FAtomRuntime;

UENUM()
enum class EAtomSpatializationType : uint8
{
	Panning,
	SendToChannel
};

UENUM()
enum class EAtomBusSendType : uint8
{
	PreVolume,
	PostVolume,
	PostPanning
};

/*
 * FAtomChannelLevelMatrix structure
 *****************************************************************************/

USTRUCT(Blueprintable)
struct CRIWARECORE_API FAtomChannelLevelMatrix
{
	GENERATED_BODY()

public:

	FAtomChannelLevelMatrix();

	/** */
	FAtomChannelLevelMatrix(int NumInputChannels, int NumOutputChannels);

	// currently speaker format seams fixed in Atom
	/*FAtomChannelLevelMatrix(const TArray<EAtomSpeaker>& InInputChannelArray, int NumOutputChannels)
	{
		InputChannelArray = InInputChannelArray;
		SetupChannelArrayFromNumChannels(OutputChannelArray, NumOutputChannels);
	}

	FAtomChannelLevelMatrix(const TArray<EAtomSpeaker>& InInputChannelArray, const TArray<EAtomSpeaker>& InOutputChannelArray)
	{
		InputChannelArray = InInputChannelArray;
		OutputChannelArray = InOutputChannelArray;
	}*/

	/** */
	bool SetChannelLevel(EAtomSpeaker InChannel, EAtomSpeaker OutChannel, float Level);

	/** */
	float GetChannelLevel(EAtomSpeaker InChannel, EAtomSpeaker OutChannel) const;

	/** */
	FORCEINLINE const float* GetLevelMatixData() const { return LevelMatrix.GetData(); }

	/** */
	FORCEINLINE int32 GetNumInputChannels() const { return InputChannelArray.Num(); }

	/** */
	FORCEINLINE int32 GetNumOutputChannels() const { return OutputChannelArray.Num(); }

	/** */
	void SetIdentity();
	
	void UpMix();
	void DownMix();

private:

	void SetupChannelArrayFromNumChannels(TArray<EAtomSpeaker>& ChannelArray, int NumChannels);

	// Layout
	//UPROPERTY()
	TArray<EAtomSpeaker> InputChannelArray;
	//UPROPERTY()
	TArray<EAtomSpeaker> OutputChannelArray;

	// Values
	UPROPERTY(EditInstanceOnly, Category = "Level Mixer")
	TArray<float> LevelMatrix;
};

/*
 * FAtomPan structure
 *****************************************************************************/

USTRUCT(BlueprintType)
struct CRIWARECORE_API FAtomPanning
{
	GENERATED_BODY()

public:

	FAtomPanning();
	FAtomPanning(float Volume, float Angle, float Distance, float Wideness, float Spread);

	/** */
	UPROPERTY(EditAnywhere, Category = "Level Mixer", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0", SliderExponent = "5"))
	float Volume;

	/** */
	UPROPERTY(EditAnywhere, Category = "Level Mixer", meta = (ClampMin = -180.f, ClampMax = 180.f, UIMin = -180.f, UIMax = 180.f, SliderExponent = "1"))
	float Angle;

	/** */
	UPROPERTY(EditAnywhere, Category = "Level Mixer", meta = (ClampMin = 0.f, ClampMax = 1.f, UIMin = 0.f, UIMax = 1.f, SliderExponent = 5))
	float Distance;

	/** */
	UPROPERTY(EditAnywhere, Category = "Level Mixer", meta = (ClampMin = 0.f, ClampMax = 1.f, UIMin = 0.f, UIMax = 1.f, SliderExponent = 5))
	float Wideness;

	/** */
	UPROPERTY(EditAnywhere, Category = "Level Mixer", meta = (ClampMin = 0.f, ClampMax = 1.f, UIMin = 0.f, UIMax = 1.f, SliderExponent = 5))
	float Spread;
};

/*
 * FAtomBusSend structure
 *****************************************************************************/

USTRUCT()
struct FAtomBusSend
{
	GENERATED_BODY()

public:
	FAtomBusSend();
	FAtomBusSend(const UAtomBus* InDestinationBus, float InLevel = 0.0f, EAtomBusSendType SendType = EAtomBusSendType::PostPanning);
	
	/** */
	UPROPERTY(VisibleAnyWhere, Category = "Send Level")
	TObjectPtr<const UAtomBus> DestinationBus;

	/** */
	UPROPERTY(EditAnywhere, Category = "Send Level", meta = (ClampMin = 0.f, ClampMax = 1.f, UIMin = 0.f, UIMax = 1.f, SliderExponent = 5))
	float Level;

#if WITH_EDITORONLY_DATA
	/** */
	UPROPERTY(VisibleAnyWhere, Category = "Send Level")
	EAtomBusSendType SendType;
#endif
};

/*
 * UAtomBus class
 *****************************************************************************/

 // Forward Declarations
class UAtomRack;

UCLASS(hidecategories = Object, BlueprintType)
class CRIWARECORE_API UAtomBus
	: public UObject
{
	GENERATED_BODY()

public:

	UAtomBus(const FObjectInitializer& ObjectInitializer);
	
	void Init(const FString& InBusName, int InBusIndex);

	//~ Begin UObject Interface
	virtual void PostInitProperties() override;
#if WITH_EDITOR

	/**
	* Add Referenced objects
	*
	* @param	InThis		AtomBus we are adding references from.
	* @param	Collector	Reference Collector
	*/
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

protected:

	//virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;

	static TArray<TObjectPtr<UAtomBusEffectPreset>> BackupBusEffectChain;
#endif
	//~ End Uobject Interface

public:

	/** The output volume of the bus in Decibels. Applied after buses effects and analysis are performed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Mixer", meta = (DisplayName = "Output Volume (dB)", AtomParam = "Volume", AtomParamClass = "AtomModulationParameterVolume", ClampMin = "-96.0", ClampMax = "0.0", UIMin = "-96.0", UIMax = "0.0"))
	FAtomSoundModulationSettings OutputVolumeModulation;

	/** The type of spatialization of the multi-channels for this bus. */
	UPROPERTY(EditAnywhere, Category = "Level Mixer", meta = (DisplayAfter = OutputVolumeModulation))
	EAtomSpatializationType SpatializationType;

	/** The level matrix for each channels for this bus. */
	UPROPERTY(EditAnywhere, Category = "Level Mixer", meta = (ShowOnlyInnerProperties, DisplayAfter=Panning, EditCondition="SpatializationType == EAtomSpatializationType::SendToChannel"))
	FAtomChannelLevelMatrix ChannelLevelMatrix;

	UPROPERTY(EditAnywhere, EditFixedSize, Category = "Level Mixer", meta = (DisplayAfter = ChannelLevelMatrix))
	TArray<FAtomBusSend> Sends;

	/** Sets the output volume of the bus in linear gain. This dynamic volume acts as a multiplier on the OutputVolume property of this bus.  */
	UFUNCTION(BlueprintCallable, Category = "Atom|Level Mixer", meta = (WorldContext = "WorldContextObject", DisplayName = "SetBusOutputVolume (linear gain)"))
	void SetBusOutputVolume(const UObject* WorldContextObject, float InOutputVolume);

	/** */
	UFUNCTION(BlueprintCallable, Category = "Atom|Level Mixer")
	float GetBusOutputVolume() const;

	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = "Atom|Level Mixer")
	void SetPanning(const FAtomPanning& PanningSetting);

	UFUNCTION(BlueprintCallable, BlueprintGetter, Category = "Atom|Level Mixer")
	FAtomPanning GetPanning() const;

	/** */
	UFUNCTION(BlueprintCallable, Category = "Atom|Level Mixer")
	void SetChannelLevel(EAtomSpeaker InChannel, EAtomSpeaker OutChannel, float Level);

	/** */
	UFUNCTION(BlueprintCallable, Category = "Atom|Level Mixer")
	float GetChannelLevel(EAtomSpeaker InChannel, EAtomSpeaker OutChannel);

	/** */
	UFUNCTION(BlueprintCallable, Category = "Atom|Level Mixer")
	void ResetChannelLevelMatrix();

	UFUNCTION(BlueprintCallable, Category = "Atom|Level Mixer")
	void SetSendLevel(int SendIndex, float Level);

	UFUNCTION(BlueprintCallable, Category = "Atom|Level Mixer")
	float GetSendLevel(int SendIndex);

	/** Sets preset to corresponding bus effect, overriding previous preset if present and retrun false when effect type is not valid with this bus. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Effects")
	bool SetEffectPreset(UAtomBusEffectPreset* InEffectPreset);

	/** Removes preset to corresponding bus effect, default bus effect settings will be restored. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Effects")
	bool RemoveEffectPreset(UAtomBusEffectPreset* InEffectPreset);

	UFUNCTION(BlueprintCallable, Category = "Atom|Effects")
	bool SetDefaultEffectPreset(int32 InEffectIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Atom|Effects")
	UAtomBusEffectPreset* GetEffectPreset(int32 InEffectIndex)
	{
		return BusEffectChain.IsValidIndex(InEffectIndex) ? BusEffectChain[InEffectIndex] : nullptr;
	}

	UFUNCTION(BlueprintCallable, Category = "Atom|Effects")
	int32 GetNumEffectPresets() { return BusEffectChain.Num(); }

	/** Finds the given preset if it is actualy used in this bus. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Effects")
	int32 FindEffectPreset(UAtomBusEffectPreset* InEffectPreset) const;

	/** Gets the Parent holder rack of this bus. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Bus")
	UAtomRackBase* GetRack() const;

	/** Whether this bus is a main bus. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Bus")
	bool IsMainBus() const { return BusIndex == 0; }

	/** Gets the bus name identifier. */
	FORCEINLINE const FString& GetBusName() const { return BusName; }

	/** Reset this bus to default values, uses DspSetting if available (Editor). */
	void Reset(bool bApplyToAtom = true);

protected:

	/** Name that identifies this bus.*/
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Bus")
	FString BusName;

	/** The panning settings for this bus. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetPanning, BlueprintGetter = GetPanning, DisplayName = "Spatial Panning", Category = "Level Mixer", meta = (DisplayAfter=SpatializationType, EditCondition="SpatializationType == EAtomSpatializationType::Panning"))
	FAtomPanning Panning;

	/** The presets classes used by this bus. (Hidden by Customization.) */
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Effects");
	TArray<TSubclassOf<UAtomBusEffectPreset>> BusEffectChainClasses;

	/** The presets to apply to bus. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, EditFixedSize, meta = (EditFixedOrder), Category = "Effects")
	TArray<TObjectPtr<UAtomBusEffectPreset>> BusEffectChain;

protected:

	/** Sync the native bus with current volume. */
	void ApplyVolume(const UObject* WorldContextObject, float InOutputVolume) const;

	/** Sync the native bus with current spatialization panning settings. */
	void ApplyPanning() const;

	/** Sync the native bus with current level matrix. */
	void ApplyChannelLevelMatrix() const;

	/** Sync the native bus with given bus send level. */
	void ApplySendLevel(const FAtomBusSend& InSend) const;

	/** Sync the native bus effect parameter with given value. */
	void ApplyEffectParameter(const UAtomBusEffectPreset* EffectPreset, int ParameterIndex) const; // todo: lookup by effect name

	/** Sync the native bus effect bypass with given value. */
	void ApplyEffectBypass(const UAtomBusEffectPreset* EffectPreset) const; // todo: lookup by effect name

	/** Sync the native bus effect with all preset settings (parameters and bypass). */
	void ApplyEffectPreset(const UAtomBusEffectPreset* EffectPreset) const; // todo: lookup by effect name

	/** Sync the native bus with all current settings from this AtomBus. */
	void ApplyAll() const;

	/** Sync this AtomBus with all possible settings from the native bus. */
	void SyncAll();

	/** Reset sends to default values. */
	void ResetSends();

	/** Reset Effect chain to defaults. */
	void ResetEffectChain(bool bUseRuntimeDefaults = false);

	/** Add an effect slot based on its name. */
	int AddEffect(const FName& InEffectName, bool bIsBypassed, bool bUseRuntimeValues);

#if WITH_EDITORONLY_DATA
	/** Setup effect preset settings from AtomConfig bus effect values. */
	void SetEditorEffectSettings(UAtomBusEffectPreset* EffectPreset);
#endif

	friend class UAtomRackWithParentBase;
	friend class UAtomRack;
	friend class UAtomBusEffectPreset;

private:

	int BusIndex = INDEX_NONE;

	float CurrentOutputVolume;
	float TargetOutputVolume;
	FAtomModulationDestination VolumeMod;
	float VolumeModBaseDb = 0.f;
	// modifiers set from BP code
	float VolumeModifier = 1.f;

	void InitInternal();
	void ProcessAudio(FAtomRuntime* AtomRuntime);
	FORCEINLINE int GetBusIndex() const { return BusIndex; }

	friend class FAtomRuntime;
	friend class Atom::FAtomExPlayback;
};
