#pragma once

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Math/Interval.h"

#include "CriWareDefines.h"
#include "AtomModulation.h"
#include "AtomModulationParameter.generated.h"

USTRUCT(BlueprintType)
struct CRIWARECORE_API FAtomModulationParameterSettings
{
	GENERATED_BODY()

	/**
		* Normalized, unitless default value of modulator. To ensure bypass functionality of mixing, patching, and modulating
		* functions as anticipated, value should be selected such that GetMixFunction (see USoundModulationParameter)
		* reduces to an identity function (i.e. function acts as a "pass-through" for all values in the range [0.0, 1.0]).
		* If GetMixFunction performs the mathematical operation f(x1, x2), then the default ValueNormalized should result in
		* f(x1, d) = x1 where d is ValueNormalized.
		*/
	UPROPERTY(EditAnywhere, Category = General, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ValueNormalized = 1.0f;

#if WITH_EDITORONLY_DATA
	/** (Optional) Text name of parameter's unit */
	UPROPERTY(EditAnywhere, Category = General)
	FText UnitDisplayName;

	/** Default value of modulator in units (editor only) */
	UPROPERTY(Transient, EditAnywhere, Category = General)
	float ValueUnit = 1.0f;
#endif // WITH_EDITORONLY_DATA
};

UCLASS(BlueprintType)
class CRIWARECORE_API UAtomModulationParameter : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = General, BlueprintReadOnly, meta = (DisplayName = "Parameter"))
	FAtomModulationParameterSettings Settings;

public:
	/** Whether or not the parameter requires a unit conversion. */
	virtual bool RequiresUnitConversion() const
	{
		return false;
	}

	/** Function used to mix modulator units together */
	virtual FAtomModulationMixFunction GetMixFunction() const
	{
		return FAtomModulationParameter::GetDefaultMixFunction();
	}

	/** Function used to convert normalized, unitless value to unit value */
	virtual FAtomModulationNormalizedConversionFunction GetUnitConversionFunction() const
	{
		return FAtomModulationParameter::GetDefaultUnitConversionFunction();
	}

	/** Function used to convert unit value to normalized, unitless value */
	virtual FAtomModulationNormalizedConversionFunction GetNormalizedConversionFunction() const
	{
		return FAtomModulationParameter::GetDefaultNormalizedConversionFunction();
	}

	/** Converts normalized, unitless value [0.0f, 1.0f] to unit value. */
	virtual float ConvertNormalizedToUnit(float InNormalizedValue) const final
	{
		float UnitValue = InNormalizedValue;
		GetUnitConversionFunction()(UnitValue);
		return UnitValue;
	}

	/** Converts unit value to unitless, normalized value [0.0f, 1.0f]. */
	virtual float ConvertUnitToNormalized(float InUnitValue) const final
	{
		float NormalizedValue = InUnitValue;
		GetNormalizedConversionFunction()(NormalizedValue);
		return NormalizedValue;
	}

	/** Returns default unit value (works with and without editor loaded) */
	virtual float GetUnitDefault() const final
	{
		return ConvertNormalizedToUnit(Settings.ValueNormalized);
	}

	virtual float GetUnitMin() const
	{
		return 0.0f;
	}

	virtual float GetUnitMax() const
	{
		return 1.0f;
	}

	//~Begin IAudioProxyDataFactory Interface.
	//virtual TUniquePtr<Audio::IProxyData> CreateNewProxyData(const Audio::FProxyDataInitParams& InitParams) override;
	//~ End IAudioProxyDataFactory Interface.

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	void RefreshNormalizedValue();
	void RefreshUnitValue();
#endif // WITH_EDITOR

	FAtomModulationParameter CreateParameter() const;
};

// Linearly scaled value between unit minimum and maximum.
UCLASS(BlueprintType, MinimalAPI)
class UAtomModulationParameterScaled : public UAtomModulationParameter
{
	GENERATED_BODY()

public:
	/** Unit minimum of modulator. Minimum is only enforced at modulation destination. */
	UPROPERTY(EditAnywhere, Category = General, BlueprintReadOnly)
	float UnitMin = 0.0f;

	/** Unit maximum of modulator. Maximum is only enforced at modulation destination. */
	UPROPERTY(EditAnywhere, Category = General, BlueprintReadOnly)
	float UnitMax = 1.0f;

	virtual bool RequiresUnitConversion() const override;
	virtual FAtomModulationUnitConversionFunction GetUnitConversionFunction() const override;
	virtual FAtomModulationUnitConversionFunction GetNormalizedConversionFunction() const override;
	virtual float GetUnitMin() const override;
	virtual float GetUnitMax() const override;
};

// Modulation Parameter that scales normalized, unitless value to logarithmic frequency unit space.
UCLASS(BlueprintType, MinimalAPI, abstract)
class UAtomModulationParameterFrequencyBase : public UAtomModulationParameter
{
	GENERATED_BODY()

public:
	virtual bool RequiresUnitConversion() const override;
	virtual FAtomModulationUnitConversionFunction GetUnitConversionFunction() const override;
	virtual FAtomModulationUnitConversionFunction GetNormalizedConversionFunction() const override;
};

// Modulation Parameter that scales normalized, unitless value to logarithmic frequency unit space with provided minimum and maximum.
UCLASS(BlueprintType, MinimalAPI)
class UAtomModulationParameterFrequency : public UAtomModulationParameterFrequencyBase
{
	GENERATED_BODY()

public:
	/** Unit minimum of modulator. Minimum is only enforced at modulation destination. */
	UPROPERTY(EditAnywhere, Category = General, BlueprintReadOnly)
	float UnitMin = ATOM_MIN_FILTER_FREQUENCY;

	/** Unit maximum of modulator. Maximum is only enforced at modulation destination. */
	UPROPERTY(EditAnywhere, Category = General, BlueprintReadOnly)
	float UnitMax = ATOM_MAX_FILTER_FREQUENCY;

	virtual float GetUnitMin() const override
	{
		return UnitMin;
	}

	virtual float GetUnitMax() const override
	{
		return UnitMax;
	}
};

// Modulation Parameter that scales normalized, unitless value to logarithmic frequency unit space with standard filter min and max frequency set.
UCLASS(BlueprintType, MinimalAPI)
class UAtomModulationParameterFilterFrequency : public UAtomModulationParameterFrequencyBase
{
	GENERATED_BODY()

public:
	virtual float GetUnitMin() const override
	{
		return ATOM_MIN_FILTER_FREQUENCY;
	}

	virtual float GetUnitMax() const override
	{
		return ATOM_MAX_FILTER_FREQUENCY;
	}
};

// Modulation Parameter that scales normalized, unitless value to logarithmic frequency unit space with standard filter min and max frequency set.
// Mixes by taking the minimum (i.e. aggressive) filter frequency of all active modulators.
UCLASS(BlueprintType, MinimalAPI)
class UAtomModulationParameterLPFFrequency : public UAtomModulationParameterFilterFrequency
{
	GENERATED_BODY()

public:
	virtual FAtomModulationMixFunction GetMixFunction() const override;
};

// Modulation Parameter that scales normalized, unitless value to logarithmic frequency unit space with standard filter min and max frequency set.
// Mixes by taking the maximum (i.e. aggressive) filter frequency of all active modulators.
UCLASS(BlueprintType, MinimalAPI)
class UAtomModulationParameterHPFFrequency : public UAtomModulationParameterFilterFrequency
{
	GENERATED_BODY()

public:
	UAtomModulationParameterHPFFrequency(const FObjectInitializer& ObjectInitializer);

	virtual FAtomModulationMixFunction GetMixFunction() const override;
};

// Modulation Parameter that scales normalized, unitless value to bipolar range. Mixes multiplicatively.
UCLASS(BlueprintType, MinimalAPI)
class UAtomModulationParameterBipolar : public UAtomModulationParameter
{
	GENERATED_BODY()

public:
	UAtomModulationParameterBipolar(const FObjectInitializer& ObjectInitializer);

	/** Unit range of modulator. Range is only enforced at modulation destination. */
	UPROPERTY(EditAnywhere, Category = General, BlueprintReadOnly, meta = (ClampMin = 0.00000001))
	float UnitRange = 2.0f;

	virtual bool RequiresUnitConversion() const override;
	virtual FAtomModulationMixFunction GetMixFunction() const override;
	virtual FAtomModulationUnitConversionFunction GetUnitConversionFunction() const override;
	virtual FAtomModulationNormalizedConversionFunction GetNormalizedConversionFunction() const override;
	virtual float GetUnitMax() const override;
	virtual float GetUnitMin() const override;
};

UCLASS(BlueprintType, MinimalAPI)
class UAtomModulationParameterVolume : public UAtomModulationParameter
{
	GENERATED_BODY()

public:
	/** Minimum volume of parameter. Only enforced at modulation destination. */
	UPROPERTY(EditAnywhere, Category = General, BlueprintReadOnly, meta = (ClampMax = 0.0))
	float MinVolume = -100.0f;

	virtual bool RequiresUnitConversion() const override;
	virtual FAtomModulationUnitConversionFunction GetUnitConversionFunction() const override;
	virtual FAtomModulationNormalizedConversionFunction GetNormalizedConversionFunction() const override;
	virtual float GetUnitMin() const override;
	virtual float GetUnitMax() const override;
};

namespace Atom
{
	//class FSoundModulationPluginParameterAssetProxy : public FSoundModulationParameterAssetProxy
	//{
	//public:
	//	explicit FSoundModulationPluginParameterAssetProxy(USoundModulationParameter* InParameter);
	//	FSoundModulationPluginParameterAssetProxy(const FSoundModulationPluginParameterAssetProxy& InProxy) = default;
//
	//	virtual Audio::IProxyDataPtr Clone() const override;
	//};

	/*
	 * Returns given registered parameter instance reference or creates it from the given asset if not registered.
	 * @param InParameter - Parameter asset associated with the pre-existing or to-create parameter
	 * @param InBreadcrumb - String identifying get or register request initiator.
	 */
	CRIWARECORE_API const FAtomModulationParameter& GetOrRegisterParameter(const UAtomModulationParameter* InParameter, const FString& InBreadcrumb);
} // namespace
