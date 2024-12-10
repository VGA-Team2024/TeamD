/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCore
 * File     : AtomBlueprintLibrary.h
 *
 ****************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "Atom.h"
#include "AtomBus.h"

#include "AtomBlueprintLibrary.generated.h"

class UAtomSoundBase;
class UAtomWaveBank;
class UAtomCueSheet;
class UAtomComponent;

/**
  * Called when a load request for a sound has completed.
  */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnAtomSoundLoadComplete, const class UAtomSoundBase*, LoadedSound, const bool, WasCancelled);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnAtomWaveBankLoadComplete, const class UAtomWaveBank*, LoadedWaveBank, const bool, WasCancelled);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnAtomCueSheetLoadComplete, const class UAtomCueSheet*, LoadedCueSheet, const bool, WasCancelled);

/** Legacy Atom category - Called when category status values are retreived from Atom runtime. */
DECLARE_DYNAMIC_DELEGATE_SixParams(FOnAtomGetCategoryStatus, const FName, CategoryName, const bool, IsPaused, const bool, IsMuted, const bool, IsSoloed, const float, Volume, const int32, PlayCount);

/** Called when game variable value is retreived from Atom runtime.*/
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnAtomGetGameVariableValue, const FName, Name, const float, Value);

UCLASS(meta = (ScriptName = "AtomRuntimeLibrary"))
class CRIWARECORE_API UAtomRuntimeBlueprintLibrary
	: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public: // Sounds Cache

	/** Begin loading a sound into the cache so that it can be played immediately. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Cache")
	static void PrimeSoundForPlayback(UAtomSoundBase* Sound, const FOnAtomSoundLoadComplete OnLoadCompletion);

	/** Begin loading any sounds referenced by a sound cue into the cache so that it can be played immediately. */
	//UFUNCTION(BlueprintCallable, Category = "Atom|Cache")
	//static void PrimeSoundCueForPlayback(UAtomSoundCue* SoundCue);

	/** Begin loading any sound cue referenced by a sound cue sheet into the cache so that it can be played immediately. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Cache")
	static void PrimeCueSheetForPlayback(UAtomCueSheet* CueSheet, const FOnAtomCueSheetLoadComplete OnLoadCompletion);

	/** Begin loading any sound wave referenced by a sound wave bank into the cache so that it can be played immediately. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Cache")
	static void PrimeWaveBankForPlayback(UAtomWaveBank* WaveBank, const FOnAtomWaveBankLoadComplete OnLoadCompletion);

public: // Bus Effects - Direct setup over current Atom runtime sound rendering.

	/** Set a bus effect preset to the master rack, master out bus. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Effects", meta = (WorldContext = "WorldContextObject"))
	static void SetMasterEffectPreset(const UObject* WorldContextObject, UAtomBusEffectPreset* BusEffectPreset);

	/** Removes a bus effect preset from the master rack, master out bus. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Effects", meta = (WorldContext = "WorldContextObject"))
	static void RemoveMasterEffectPreset(const UObject* WorldContextObject, UAtomBusEffectPreset* BusEffectPreset);

	/** Clears all effect presets from the master rack, master out bus. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Effects", meta = (WorldContext = "WorldContextObject"))
	static void ClearMasterEffectPresets(const UObject* WorldContextObject);

	/** Set a bus effect preset to the given bus. Returns the index of the effect in the bus or index none if not found. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Effects", meta = (WorldContext = "WorldContextObject"))
	static int32 SetBusEffectPreset(const UObject* WorldContextObject, UAtomBus* AtomBus, UAtomBusEffectPreset* BusEffectPreset);

	/** Removes the bus effect preset at the given bus, given effect index, if there is an effect preset set at that index. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Effects", meta = (WorldContext = "WorldContextObject"))
	static void RemoveBusEffectPreset(const UObject* WorldContextObject, UAtomBus* AtomBus, int32 EffectIndex);

	/** Clears all submix effect perests on the given bus. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Effects", meta = (WorldContext = "WorldContextObject"))
	static void ClearBusEffectPresets(const UObject* WorldContextObject, UAtomBus* AtomBus);

	/** Removes all instances of a bus effect preset from any buses of the given Atom rack. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Effects", meta = (WorldContext = "WorldContextObject"))
	static void RemoveRackEffectPreset(const UObject* WorldContextObject, UAtomRack* AtomRack, UAtomBusEffectPreset* BusEffectPreset);

	/** Clears all effect presets from any buses of the given rack. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Effects", meta = (WorldContext = "WorldContextObject"))
	static void ClearRackEffectPresets(const UObject* WorldContextObject, UAtomRack* AtomRack);

	/** Enable or disable binauralizer for all Atom soundfield racks. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Effects", meta = (WorldContext = "WorldContextObject"))
	static void EnableBinauralizer(const UObject* WorldContextObject, bool bEnable);

	/** Gets whether binauralizer is enabled or disabled for all Atom soundfield racks. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Effects", meta = (WorldContext = "WorldContextObject"))
	static bool IsBinauralizerEnabled(const UObject* WorldContextObject);

public: // Legacy Atom category to control categories set in each cue by a sound designer in Atom Craft.

	/** Legacy Atom category - Stops all sounds using this category in current Atom runtime. (This by-pass all Unreal Engine gameplay layers.) */
	UFUNCTION(BlueprintCallable, Category = "Atom|Category|Legacy", meta = (WorldContext = "WorldContextObject"))
	static void StopCategoryByName(const UObject* WorldContextObject, FName CategoryName);

	/** Legacy Atom category - Pauses all sounds using this category in current Atom runtime. (This by-pass all Unreal Engine gameplay layers.) */
	UFUNCTION(BlueprintCallable, Category = "Atom|Category|Legacy", meta = (WorldContext = "WorldContextObject"))
	static void PauseCategoryByName(const UObject* WorldContextObject, FName CategoryName, bool bPause);

	/** Legacy Atom category - Mutes all sounds using this category in current Atom runtime. (This by-pass all Unreal Engine gameplay layers.) */
	UFUNCTION(BlueprintCallable, Category = "Atom|Category|Legacy", meta = (WorldContext = "WorldContextObject"))
	static void MuteCategoryByName(const UObject* WorldContextObject, FName CategoryName, bool bMute);

	/** Legacy Atom category - Solo all sounds using this category in current Atom runtime and mute other categories using the given volume value. (This by-pass all Unreal Engine gameplay layers.) */
	UFUNCTION(BlueprintCallable, Category = "Atom|Category|Legacy", meta = (WorldContext = "WorldContextObject"))
	static void SoloCategoryByName(const UObject* WorldContextObject, FName CategoryName, bool bSolo, float MuteVolume);

	/** Legacy Atom category - Sets category volume to current Atom runtime. (This by-pass all Unreal Engine gameplay layers.) */
	UFUNCTION(BlueprintCallable, Category = "Atom|Category|Legacy", meta = (WorldContext = "WorldContextObject"))
	static void SetCategoryVolumeByName(const UObject* WorldContextObject, FName CategoryName, float VolumeMultiplier);

	/** Legacy Atom category - Retrieves category status values from Atom runtime. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Category|Legacy", meta = (WorldContext = "WorldContextObject"))
	static void GetCategoryStatusByName(const UObject* WorldContextObject, FName CategoryName, const FOnAtomGetCategoryStatus OnGetStatus);

public: // Atom Game Variables

	/** Sets value of an Atom Game Variable in current Atom runtime. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Game Variables", meta = (WorldContext = "WorldContextObject"))
	static void SetGameVariableValueByName(const UObject* WorldContextObject, FName GameVariableName, float Value);

	/** Retrives value of an Atom Game Variable from Atom runtime. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Game Variables", meta = (WorldContext = "WorldContextObject"))
	static void GetGameVariableValueByName(const UObject* WorldContextObject, FName CategoryName, const FOnAtomGetGameVariableValue OnGetValue);

public: // Atom Output Ports

	/** Maps a named output port to existing rack (default mapping is master rack.) [Experimental] */
	UFUNCTION(BlueprintCallable, Category = "Atom|Output Ports", meta = (WorldContext = "WorldContextObject"))
	static bool MapOutputPortToRack(const UObject* WorldContextObject, FName OutputPortName, UAtomRackWithParentBase* AtomRack);

public: // Atom playback API for blueprint node based cues (Experimental)
	
	/** Lists all playabacks currently controllable from a given AtomComponent. (Experimental) */
	UFUNCTION(BlueprintCallable, Category = "Atom|Playbacks")
	static void GetAllPlaybacks(const UAtomComponent* AtomComponent, TArray<FAtomSoundPlayback>& Playbacks);

	/** Stops the given playback. This may run a new playback if Stop Actions are setup with this playback. (Experimental) */
	UFUNCTION(BlueprintCallable, Category = "Atom|Playbacks")
	static void StopPlayback(const UAtomComponent* AtomComponent, FAtomSoundPlayback PlaybackToStop);

	/* Playback Node API - WIP
	void CreatePlaybackFromSound(const UAtomComponent* AtomComponent, const UAtomSoundBase* Sound);
	void CreatePlaybackFromPlayback(const UAtomComponent* AtomComponent, int64 Playback);
	void PlayPlayback(const UAtomComponent* AtomComponent, const UAtomSoundBase* Sound, float StartTimeInMs);
	void PausePlayback(const UAtomComponent* AtomComponent, int64 PlaybackToPause, bool bPause);

	void SetPlaybackAttenuation(const UAtomComponent* AtomComponent, int64 Playback, const UAtomPlaybackAttenuation* Attenuation);
	void SetPlaybackVolume(const UAtomComponent* AtomComponent, int64 Playback, float Vol);
	void SetPlaybackPitch(const UAtomComponent* AtomComponent, int64 Playback, float Pitch);
	void SetPlaybackLowPass(const UAtomComponent* AtomComponent, int64 Playback, float LowPass);
	void SetPlaybackHighPass(const UAtomComponent* AtomComponent, int64 Playback, float HighPass);
	void SetPlayabackModulationAISAC(const UAtomComponent* AtomComponent, int64 Playback, UAtomModulatorBase* Modulator);*/
};

/** Atom Utilities Blueprint Function Library
 *  A library of Atom related functions for use in Blueprints
 */
UCLASS(meta = (ScriptName = "AtomUtilitiesLibrary"))
class CRIWARECORE_API UAtomUtilitiesBlueprintLibrary
	: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Converts linear scale volume to decibels.
	 * @param InLinear		The linear scalar value to convert to decibels.
	 * @param InRange		The range to use when converting between linear and logarithmic scales.
	 * @param InFloor		The floor value to check against.
	 * @return				The decibel value of the inputted linear scale.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Utilities", meta = (AdvancedDisplay = 2))
	static float ConvertLinearToDecibels(float InLinear, const FVector2D& InRange, float InFloor = 1.e-8/*SMALL_NUMBER*/);

	/**
	 * Converts decibel to linear scale.
	 * @param InDecibels	The decibels to convert to a linear gain scalar.
	 * @param InRange		The range to use when converting between linear and logarithmic scales.
	 * @return				The resulting linear gain.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Utilities")
	static float ConvertDecibelsToLinear(float InDecibels, const FVector2D& InRange);

	/**
	 * Converts linear scale volume to decibels with a slower end from 1.e-3 (60dB).
	 * @param InLinear		The linear scalar value to convert to decibels.
	 * @param InRange		The range to use when converting between linear and logarithmic scales.
	 * @return				The decibel value of the inputted linear scale.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Utilities")
	static float ConvertLinearToDecibelsSlower(float InLinear, const FVector2D& InRange);

	/**
	 * Converts decibel to linear scale with a slower end from 1.e-3 (60dB).
	 * @param InDecibels	The decibels to convert to a linear gain scalar.
	 * @param InRange		The range to use when converting between linear and logarithmic scales.
	 * @return				The resulting linear gain.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Utilities")
	static float ConvertDecibelsToLinearSlower(float InDecibels, const FVector2D& InRange);

	/**
	 * Converts linear scale volume to decibels with a slower end from a choosen floor.
	 * @param InLinear		The linear scalar value to convert to decibels.
	 * @param InRange		The range to use when converting between linear and logarithmic scales.
	 * @param InFloor		The floor value to start slower end. (1.e-3 => 60dB, 1.e-4 => 80dB ...)
	 * @return				The decibel value of the inputted linear scale.
	 */
	// Transform a normalized value [0, 1] to decibel(dB) value in range[Min, Max] using logarithmic scale weigthed with a dynamic range in decibels (dB).
	UFUNCTION(BlueprintCallable, Category = "Atom|Utilities", meta = (AdvancedDisplay = 2))
	static float ConvertLinearToDecibelsSlowerFromFloor(float InLinear, const FVector2D& InRange, float InFloor = 1.e-4/*KINDA_SMALL_NUMBER*/);

	/**
	 * Converts decibel to linear scale with a slower end from a choosen floor.
	 * @param InDecibels	The decibels to convert to a linear gain scalar.
	 * @param InRange		The range to use when converting between linear and logarithmic scales.
	 * @param InFloor		The floor value to start slower end. (1.e-3 => 60dB, 1.e-4 => 80dB ...)
	 * @return				The resulting linear gain.
	 */
	// Transform a decibel (dB) value in range [Min, Max] to normalized value [0, 1] using logarithmic scale weigthed with a dynamic range in decibels (dB).
	UFUNCTION(BlueprintCallable, Category = "Atom|Utilities", meta = (AdvancedDisplay = 2))
	static float ConvertDecibelsToLinearSlowerFromFloor(float InDecibels, const FVector2D& InRange, float InFloor = 1.e-4/*KINDA_SMALL_NUMBER*/);

	/** 
	 * Returns the log frequency of the input value. Maps linear domain and range values to log output (good for linear slider controlling frequency) 
	 * @param InValue					The linear value to convert to logarithmic frequency
	 * @param InDomain					The domain to use when converting between linear and logarithmic scales
	 * @param InRange					The range to use when converting between linear and logarithmic scales
	 * @return							The log frequency of the given input
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Utilities")
	static float GetLogFrequencyClamped(const float InValue, const FVector2D& InDomain, const FVector2D& InRange);

	/**
	 * Returns the linear frequency of the input value. Maps log domain and range values to linear output (good for linear 
	 * slider representation/visualization of log frequency). Reverse of GetLogFrequencyClamped. 
	 * @param InValue					The logarithmic value to convert to linear frequency
	 * @param InDomain					The domain to use when converting between linear and logarithmic scales
	 * @param InRange					The range to use when converting between linear and logarithmic scales
	 * @return							The linear frequency of the given logarithmic input
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Utilities")
	static float GetLinearFrequencyClamped(const float InValue, const FVector2D& InDomain, const FVector2D& InRange);
};
