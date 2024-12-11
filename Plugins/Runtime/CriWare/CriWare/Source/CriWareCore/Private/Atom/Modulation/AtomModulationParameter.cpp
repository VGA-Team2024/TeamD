#include "Atom/Modulation/AtomModulationParameter.h"

#include "Atom/AtomRuntime.h"
#include "Atom/AtomRuntimeManager.h"
#include "Atom/Modulation/AtomModulation.h"
#include "Atom/Modulation/AtomModulationLogs.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AtomModulationParameter)

//TUniquePtr<Audio::IProxyData> UAtomModulationParameter::CreateNewProxyData(const Audio::FProxyDataInitParams& InitParams)
//{
//	using namespace AudioModulation;
//	return MakeUnique<FSoundModulationPluginParameterAssetProxy>(this);
//}

#if WITH_EDITOR
void UAtomModulationParameter::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		const FName AssetName = GetFName();
		if (Atom::IsModulationParameterRegistered(AssetName))
		{
			FAtomModulationParameter NewParam = CreateParameter();
			Atom::RegisterModulationParameter(NewParam.ParameterName, MoveTemp(NewParam));
		}
	}
}

void UAtomModulationParameter::RefreshNormalizedValue()
{
	const float NewNormalizedValue = ConvertUnitToNormalized(Settings.ValueUnit);
	const float NewNormalizedValueClamped = FMath::Clamp(NewNormalizedValue, 0.0f, 1.0f);
	if (!FMath::IsNearlyEqual(NewNormalizedValueClamped, Settings.ValueNormalized))
	{
		Settings.ValueNormalized = NewNormalizedValueClamped;
	}
}

void UAtomModulationParameter::RefreshUnitValue()
{
	const float NewUnitValue = ConvertNormalizedToUnit(Settings.ValueNormalized);
	const float NewUnitValueClamped = FMath::Clamp(NewUnitValue, GetUnitMin(), GetUnitMax());
	if (!FMath::IsNearlyEqual(NewUnitValueClamped, Settings.ValueUnit))
	{
		Settings.ValueUnit = NewUnitValueClamped;
	}
}
#endif // WITH_EDITOR

FAtomModulationParameter UAtomModulationParameter::CreateParameter() const
{
	FAtomModulationParameter Parameter;
	Parameter.ParameterName = GetFName();
	Parameter.bRequiresConversion = RequiresUnitConversion();
	Parameter.MixFunction = GetMixFunction();
	Parameter.UnitFunction = GetUnitConversionFunction();
	Parameter.NormalizedFunction = GetNormalizedConversionFunction();
	Parameter.DefaultValue = GetUnitDefault();
	Parameter.MinValue = GetUnitMin();
	Parameter.MaxValue = GetUnitMax();

#if WITH_EDITORONLY_DATA
	Parameter.UnitDisplayName = Settings.UnitDisplayName;
	Parameter.ClassName = GetClass()->GetFName();
#endif // WITH_EDITORONLY_DATA

	return Parameter;
}

bool UAtomModulationParameterFrequencyBase::RequiresUnitConversion() const
{
	return true;
}

FAtomModulationUnitConversionFunction UAtomModulationParameterFrequencyBase::GetUnitConversionFunction() const
{
	return [InUnitMin = GetUnitMin(), InUnitMax = GetUnitMax()](float& InOutValue)
	{
		static const FVector2D Domain(0.0f, 1.0f);
		const FVector2D Range(InUnitMin, InUnitMax);
		InOutValue = Atom::GetLogFrequencyClamped(InOutValue, Domain, Range);
	};
}

FAtomModulationNormalizedConversionFunction UAtomModulationParameterFrequencyBase::GetNormalizedConversionFunction() const
{
	return [InUnitMin = GetUnitMin(), InUnitMax = GetUnitMax()](float& InOutValue)
	{
		static const FVector2D Domain(0.0f, 1.0f);
		const FVector2D Range(InUnitMin, InUnitMax);
		InOutValue = Atom::GetLinearFrequencyClamped(InOutValue, Domain, Range);
	};
}

FAtomModulationMixFunction UAtomModulationParameterHPFFrequency::GetMixFunction() const
{
	return [](float& InOutValue, float InValue)
	{
		InOutValue = FMath::Max(InOutValue, InValue);
	};
}

UAtomModulationParameterHPFFrequency::UAtomModulationParameterHPFFrequency(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Settings.ValueNormalized = 0.0f;

#if WITH_EDITORONLY_DATA
	Settings.ValueUnit = GetUnitDefault();
#endif // WITH_EDITORONLY_DATA
}

FAtomModulationMixFunction UAtomModulationParameterLPFFrequency::GetMixFunction() const
{
	return [](float& InOutValueA, float InValueB)
	{
		InOutValueA = FMath::Min(InOutValueA, InValueB);
	};
}

bool UAtomModulationParameterScaled::RequiresUnitConversion() const
{
	return true;
}

FAtomModulationUnitConversionFunction UAtomModulationParameterScaled::GetUnitConversionFunction() const
{
	return [InUnitMin = UnitMin, InUnitMax = UnitMax](float& InOutValue)
	{
		InOutValue = FMath::Lerp(InUnitMin, InUnitMax, InOutValue);
	};
}

FAtomModulationNormalizedConversionFunction UAtomModulationParameterScaled::GetNormalizedConversionFunction() const
{
	return [InUnitMin = UnitMin, InUnitMax = UnitMax](float& InOutValue)
	{
		const float Denom = FMath::Max(SMALL_NUMBER, InUnitMax - InUnitMin);
		InOutValue = (InOutValue - InUnitMin) / Denom;
	};
}

float UAtomModulationParameterScaled::GetUnitMin() const
{
	return UnitMin;
}

float UAtomModulationParameterScaled::GetUnitMax() const
{
	return UnitMax;
}

UAtomModulationParameterBipolar::UAtomModulationParameterBipolar(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Settings.ValueNormalized = 0.5f;

#if WITH_EDITORONLY_DATA
	Settings.ValueUnit = GetUnitDefault();
#endif // WITH_EDITORONLY_DATA
}

bool UAtomModulationParameterBipolar::RequiresUnitConversion() const
{
	return true;
}

FAtomModulationMixFunction UAtomModulationParameterBipolar::GetMixFunction() const
{
	return [](float& InOutValueA, float InValueB)
	{
		InOutValueA += InValueB - 0.5f;
	};
}

FAtomModulationUnitConversionFunction UAtomModulationParameterBipolar::GetUnitConversionFunction() const
{
	return [InUnitRange = UnitRange](float& InOutValue)
	{
		InOutValue = (InUnitRange * InOutValue) - (0.5f * InUnitRange);
	};
}

FAtomModulationNormalizedConversionFunction UAtomModulationParameterBipolar::GetNormalizedConversionFunction() const
{
	return [InUnitRange = UnitRange](float& InOutValue)
	{
		InOutValue = 0.5f + (InOutValue / FMath::Max(InUnitRange, SMALL_NUMBER));
	};
}

float UAtomModulationParameterBipolar::GetUnitMax() const
{
	return UnitRange * 0.5f;
}

float UAtomModulationParameterBipolar::GetUnitMin() const
{
	return UnitRange * -0.5f;
}

bool UAtomModulationParameterVolume::RequiresUnitConversion() const
{
	return true;
}

FAtomModulationUnitConversionFunction UAtomModulationParameterVolume::GetUnitConversionFunction() const
{
	return [InUnitMin = GetUnitMin()](float& InOutValue)
	{
		InOutValue = InOutValue > 0.0f
			? Atom::ConvertToDecibels(InOutValue)
			: InUnitMin;
	};
}

FAtomModulationNormalizedConversionFunction UAtomModulationParameterVolume::GetNormalizedConversionFunction() const
{
	return [InUnitMin = GetUnitMin()](float& InOutValue)
	{
		InOutValue = InOutValue < InUnitMin || FMath::IsNearlyEqual(InOutValue, InUnitMin)
			? 0.0f
			: Atom::ConvertToLinear(InOutValue);
	};
}

float UAtomModulationParameterVolume::GetUnitMin() const
{
	return MinVolume;
}

float UAtomModulationParameterVolume::GetUnitMax() const
{
	return 0.0f;
}

namespace Atom
{
	const FAtomModulationParameter& GetOrRegisterParameter(const UAtomModulationParameter* InParameter, const FString& InBreadcrumb)
	{
		FName ParamName;
		if (InParameter)
		{
			ParamName = InParameter->GetFName();
			if (!Atom::IsModulationParameterRegistered(ParamName))
			{
				UE_LOG(LogAtomModulation, Display,
					TEXT("Parameter '%s' not registered.  Registration forced via '%s'."),
					*ParamName.ToString(),
					*InBreadcrumb);

				Atom::RegisterModulationParameter(ParamName, InParameter->CreateParameter());
			}
		}

		// Returns default modulation parameter if no parameter provided.
		return Atom::GetModulationParameter(ParamName);
	}
} // namespace