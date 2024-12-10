#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/ScriptMacros.h"

#include "AtomModulation.generated.h"

// Cycle stats for Atom modulation system
DECLARE_STATS_GROUP(TEXT("AtomModulation"), STATGROUP_AtomModulation, STATCAT_Advanced);

// Tracks the time for the full render block 
DECLARE_CYCLE_STAT_EXTERN(TEXT("Process Modulators"), STAT_AtomModulationProcessModulators, STATGROUP_AtomModulation, CRIWARECORE_API);

// Forward Declarations
class FAtomModulationSystem;

using FAtomModulatorId = uint32;
using FAtomModulatorTypeId = uint32;
using FAtomModulatorHandleId = uint32;

using FAtomModulationUnitConversionFunction = TFunction<void(float& /* OutValueNormalizedToUnit */)>;
using FAtomModulationNormalizedConversionFunction = TFunction<void(float& /* OutValueUnitToNormalized */)>;
using FAtomModulationMixFunction = TFunction<void(float& /* OutNormalizedA */, float /* InNormalizedB */)>;

struct CRIWARECORE_API FAtomModulationParameter
{
	FAtomModulationParameter();
	FAtomModulationParameter(const FAtomModulationParameter& InParam);
	FAtomModulationParameter(FAtomModulationParameter&& InParam);

	FAtomModulationParameter& operator=(FAtomModulationParameter&& InParam);
	FAtomModulationParameter& operator=(const FAtomModulationParameter& InParam);

	FName ParameterName;

	// Default value of parameter in unit space
	float DefaultValue = 1.0f;

	// Default minimum value of parameter in unit space
	float MinValue = 0.0f;

	// Default minimum value of parameter in unit space
	float MaxValue = 1.0f;

	// Whether or not unit conversion is required
	bool bRequiresConversion = false;

#if WITH_EDITORONLY_DATA
	FText UnitDisplayName;

	FName ClassName;
#endif // WITH_EDITORONLY_DATA

	// Function used to mix normalized values together.
	FAtomModulationMixFunction MixFunction;

	// Function used to convert value buffer from normalized, unitless space [0.0f, 1.0f] to unit space.
	FAtomModulationUnitConversionFunction UnitFunction;

	// Function used to convert value buffer from unit space to normalized, unitless [0.0f, 1.0f] space.
	FAtomModulationNormalizedConversionFunction NormalizedFunction;

	static const FAtomModulationMixFunction& GetDefaultMixFunction();
	static const FAtomModulationUnitConversionFunction& GetDefaultUnitConversionFunction();
	static const FAtomModulationNormalizedConversionFunction& GetDefaultNormalizedConversionFunction();
};

namespace Atom
{
	CRIWARECORE_API bool IsModulationParameterRegistered(FName InName);
	CRIWARECORE_API void RegisterModulationParameter(FName InName, FAtomModulationParameter&& InParameter);
	CRIWARECORE_API bool UnregisterModulationParameter(FName InName);
	CRIWARECORE_API void UnregisterAllModulationParameters();
	CRIWARECORE_API const FAtomModulationParameter& GetModulationParameter(FName InName);
	CRIWARECORE_API void IterateOverAllModulationSystems(TFunctionRef<void(FAtomModulationSystem&)> InFunction);
}

/** Interface for cached off Modulator UObject data used as default settings to
  * be converted to instanced proxy data per Atom runtime on the AtomRenderThread.
  * If proxy is already active, implementation is expected to ignore register call
  * and return existing modulator proxy's type Id & set parameter accordingly.
  */
class CRIWARECORE_API IAtomModulatorSettings
{
public:
	virtual ~IAtomModulatorSettings() = default;
	virtual TUniquePtr<IAtomModulatorSettings> Clone() const = 0;
	virtual FAtomModulatorId GetModulatorId() const = 0;
	virtual const FAtomModulationParameter& GetOutputParameter() const = 0;
	virtual FAtomModulatorTypeId Register(
		FAtomModulatorHandleId HandleId,
		FAtomModulationSystem& InModulation) const = 0;
};

/** Handle to a modulator which interacts with the modulation API to manage lifetime
  * of modulator proxy objects internal to modulation plugin implementation.
  */
struct CRIWARECORE_API FAtomModulatorHandle
{
	FAtomModulatorHandle() = default;
	FAtomModulatorHandle(FAtomModulationParameter&& InParameter);
	FAtomModulatorHandle(FAtomModulationSystem& InModulation, const IAtomModulatorSettings& InModulatorSettings, FAtomModulationParameter&& InParameter);
	FAtomModulatorHandle(const FAtomModulatorHandle& InOther);
	FAtomModulatorHandle(FAtomModulatorHandle&& InOther);

	~FAtomModulatorHandle();

	FAtomModulatorHandle& operator=(const FAtomModulatorHandle& InOther);
	FAtomModulatorHandle& operator=(FAtomModulatorHandle&& InOther);

	FAtomModulatorId GetModulatorId() const;
	const FAtomModulationParameter& GetParameter() const;
	FAtomModulatorTypeId GetTypeId() const;
	FAtomModulatorHandleId GetHandleId() const;
	bool GetValue(float& OutValue) const;
	bool GetValueThreadSafe(float& OutValue) const;
	bool IsValid() const;

	friend FORCEINLINE uint32 GetTypeHash(const FAtomModulatorHandle& InModulatorHandle)
	{
		return HashCombineFast(InModulatorHandle.HandleId, InModulatorHandle.ModulatorId);
	}

	FORCEINLINE bool operator==(const FAtomModulatorHandle& Other) const
	{
		return HandleId == Other.HandleId && ModulatorId == Other.ModulatorId;
	}

	FORCEINLINE bool operator!=(const FAtomModulatorHandle& Other) const
	{
		return !(*this == Other);
	}

private:
	FAtomModulationParameter Parameter;
	FAtomModulatorHandleId HandleId = INDEX_NONE;
	FAtomModulatorTypeId ModulatorTypeId = INDEX_NONE;
	FAtomModulatorId ModulatorId = INDEX_NONE;
	TWeakPtr<FAtomModulationSystem> Modulation;
};

/**
 * Base class for all modulators used with ADX Atom.
 */
UCLASS(Config = CriWare, abstract, EditInLineNew, BlueprintType)
class CRIWARECORE_API UAtomModulatorBase
	: public UObject
{
	GENERATED_BODY()

public:

	virtual const FAtomModulationParameter& GetOutputParameter() const;

	//virtual TUniquePtr<Audio::IProxyData> CreateNewProxyData(const Audio::FProxyDataInitParams& InitParams) override;

	virtual TUniquePtr<IAtomModulatorSettings> CreateProxySettings() const;
};
