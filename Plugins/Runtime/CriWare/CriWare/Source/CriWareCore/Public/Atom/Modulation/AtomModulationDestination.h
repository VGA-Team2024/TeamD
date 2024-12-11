
#pragma once

#include "UObject/NameTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectPtr.h"
#include "Containers/ArrayView.h"
#include "Containers/Set.h"
#include "HAL/CriticalSection.h"

#include "Atom/Atom.h"
#include "AtomModulation.h"
#include "AtomSoundModulation.h"
#include "AtomAisacModulation.h"

#include "AtomModulationDestination.generated.h"

// Forward Declarations
class UAtomModulatorBase;
struct FAtomModulationParameter;
class UAtomAisacPatch;

/*
 * Modulation destination settings - defines various structures an enums to setup modulation destinations.
 */

 /** Enumeration for diffent moduation routing for a modulation destination of Atom sound. */
UENUM(BlueprintType)
enum class EAtomModulationRouting : uint8
{
	/* Disables modulation routing. */
	Disable,

	/* Inherits modulation routing (AtomComponent inherits from Sound, Sound inherits from SoundClass). */
	Inherit,

	/* Ignores inherited settings and uses modulation settings on this object. */
	Override,

	/* Performs set union on local modulation sources with those inherited (AtomComponent inherits from Sound, Sound inherits from SoundClass). */
	Union,

	/* Ignores modulations and values in UnrealEngine and use the original settings from DAW (CRI Atom Craft) set by sound designer. */
	DAW UMETA(DisplayName="DAW - Atom Craft")
};

/** Structure allowing modulation override for Atom sound. */
USTRUCT(BlueprintType)
struct CRIWARECORE_API FAtomSoundModulationDefaultSettings
{
	GENERATED_BODY()

public:

	FAtomSoundModulationDefaultSettings();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Modulation, meta = (DisplayName = "AISAC"))
	FAtomAisacModulationDefaultSettings AisacModulations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Modulation, meta = (DisplayName = "Volume", AtomParam = "Volume", AtomParamClass = "AtomModulationParameterVolume"))
	FAtomSoundModulationSettings VolumeModulation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Modulation, meta = (DisplayName = "Pitch", AtomParam = "Pitch", AtomParamClass = "AtomModulationParameterBipolar"))
	FAtomSoundModulationSettings PitchModulation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Modulation, meta = (DisplayName = "Highpass", AtomParam = "HPFCutoffFrequency", AtomParamClass = "AtomModulationParameterHPFFrequency"))
	FAtomSoundModulationSettings HighpassModulation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Modulation, meta = (DisplayName = "Lowpass", AtomParam = "LPFCutoffFrequency", AtomParamClass = "AtomModulationParameterLPFFrequency"))
	FAtomSoundModulationSettings LowpassModulation;
};

/** Structure allowing modulation routing and override for source Atom sound. */
USTRUCT(BlueprintType)
struct CRIWARECORE_API FAtomSoundModulationRoutingSettings
	: public FAtomSoundModulationDefaultSettings
{
	GENERATED_BODY()

	FAtomSoundModulationRoutingSettings();

	/** What volume modulation settings to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Modulation)
	EAtomModulationRouting VolumeRouting = EAtomModulationRouting::Inherit;

	/** What pitch modulation settings to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Modulation)
	EAtomModulationRouting PitchRouting = EAtomModulationRouting::Inherit;

	/** What low-pass modulation settings to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Modulation)
	EAtomModulationRouting LowpassRouting = EAtomModulationRouting::Inherit;

	/** What high-pass modulation settings to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Modulation)
	EAtomModulationRouting HighpassRouting = EAtomModulationRouting::Inherit;
};

struct CRIWARECORE_API FAtomModulationDestination
{
public:
	
	FAtomModulationDestination() = default;

	FAtomModulationDestination(const FAtomModulationDestination& InModulationDestination);
	FAtomModulationDestination(FAtomModulationDestination&& InModulationDestination);

	FAtomModulationDestination& operator=(const FAtomModulationDestination& InModulationDestination);
	FAtomModulationDestination& operator=(FAtomModulationDestination&& InModulationDestination);

	/** Initializes the modulation destination
		* InRuntimeId - Atom runtime associated with modulation plugin instance
		* bInIsBuffered - Whether or not to run destination in "buffered mode," which manages an internal buffer to smooth modulation value between process calls
		* bInValueNormalized - Whether or not to keep the output value in normalized, unitless [0.0f, 1.0f] space
		*/
	void Init(FAtomRuntimeId InRuntimeID, bool bInIsBuffered = false, bool bInValueNormalized = false);

	/** Initializes the modulation destination
		* InRuntimeId - Atom runtime associated with modulation plugin instance
		* InParameterName - Name of parameter used to mix/convert destination value to/from unit space
		* bInIsBuffered - Whether or not to run destination in "buffered mode," which manages an internal buffer to smooth modulation value between process calls
		* bInValueNormalized - Whether or not to keep the output value in normalized, unitless [0.0f, 1.0f] space
		*/
	void Init(FAtomRuntimeId InRuntimeID, FName InParameterName, bool bInIsBuffered = false, bool bInValueNormalized = false);

	/** returns whether or not destination references an active modulator */
	bool IsActive();

	/* Updates internal value (or buffer if set to bIsBuffered) to current modulated result using the provided value as the base carrier value to modulate.
		* Returns true if value was updated.
		*/
	bool ProcessControl(float InValueUnitBase, int32 InNumSamples = 0);

	void UpdateModulators(const TSet<TObjectPtr<UAtomModulatorBase>>& InModulators);
	void UpdateModulators(const TSet<UAtomModulatorBase*>& InModulators);
	void UpdateModulators(const TSet<const UAtomModulatorBase*>& InModulators);
	
private:

	void UpdateModulatorsInternal(TArray<TUniquePtr<IAtomModulatorSettings>>&& ProxySettings);

	void ResetHandles();

	FAtomRuntimeId RuntimeID = INDEX_NONE;

	float ValueTarget = 1.0f;

	bool bIsBuffered = false;
	bool bValueNormalized = false;
	bool bHasProcessed = false;

	Atom::FAlignedFloatBuffer OutputBuffer;

	TSet<FAtomModulatorHandle> Handles;

	FAtomModulationParameter Parameter;

	mutable FCriticalSection HandleCritSection;

public:
	/** Returns buffer of interpolated modulation values. If not set to "IsBuffered" when initialized, returns an empty array. */
	FORCEINLINE const Atom::FAlignedFloatBuffer& GetBuffer() const
	{
		return OutputBuffer;
	}

	/** Returns whether or not the destination has requested to 
		* process the control or not. */
	FORCEINLINE bool GetHasProcessed() const
	{
		return bHasProcessed;
	}

	/** Returns sample value last reported by modulator. Returns value in unit space, unless
		* 'ValueNormalized' option is set on initialization.
		*/
	FORCEINLINE float GetValue() const
	{
		return ValueTarget;
	}
};