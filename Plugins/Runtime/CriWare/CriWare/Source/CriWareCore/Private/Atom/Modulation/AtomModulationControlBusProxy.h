#pragma once

#include "CoreMinimal.h"

#include "Atom/AtomRuntimeManager.h"
#include "Atom/Modulation/AtomModulation.h"
#include "Atom/Modulation/AtomModulationControlBus.h"
#include "Atom/Modulation/AtomModulationParameter.h"
#include "AtomModulationProxy.h"
#include "AtomModulationGeneratorProxy.h"

// Forward Declarations
class FAtomModulationSystem;

using FAtomModulationBusId = uint32;
extern const FAtomModulationBusId InvalidBusId;

struct FAtomModulationControlBusSettings : public TAtomModulatorBase<FAtomModulationBusId>, public IAtomModulatorSettings
{
	bool bBypass;
	float DefaultValue;

	TArray<FAtomModulationGeneratorSettings> GeneratorSettings;
	FAtomModulationMixFunction MixFunction;
	FAtomModulationParameter OutputParameter;

	FAtomModulationControlBusSettings(const UAtomModulationControlBus& InBus)
		: TAtomModulatorBase<FAtomModulationBusId>(InBus.GetName(), InBus.GetUniqueID())
		, bBypass(InBus.bBypass)
		, DefaultValue(InBus.GetDefaultNormalizedValue())
		, MixFunction(InBus.GetMixFunction())
		, OutputParameter(InBus.GetOutputParameter())
	{
		for (const UAtomModulationGenerator* Generator : InBus.Generators)
		{
			if (Generator)
			{
				FAtomModulationGeneratorSettings Settings(*Generator);
				GeneratorSettings.Add(MoveTemp(Settings));
			}
		}
	}

	virtual TUniquePtr<IAtomModulatorSettings> Clone() const override
	{
		return TUniquePtr<IAtomModulatorSettings>(new FAtomModulationControlBusSettings(*this));
	}

	virtual FAtomModulatorId GetModulatorId() const override
	{
		return static_cast<FAtomModulatorId>(GetId());
	}

	virtual const FAtomModulationParameter& GetOutputParameter() const override
	{
		return OutputParameter;
	}

	virtual FAtomModulatorTypeId Register(FAtomModulatorHandleId HandleId, FAtomModulationSystem& InModulation) const override;
};

class FAtomModulationControlBusProxy : public TAtomModulatorProxyRefType<FAtomModulationBusId, FAtomModulationControlBusProxy, FAtomModulationControlBusSettings>
{
public:
	FAtomModulationControlBusProxy();
	FAtomModulationControlBusProxy(FAtomModulationControlBusSettings&& InSettings, FAtomModulationSystem& InModSystem);

	FAtomModulationControlBusProxy& operator =(FAtomModulationControlBusSettings&& InSettings);

	float GetDefaultValue() const;
	const TArray<FAtomModulationGeneratorHandle>& GetGeneratorHandles() const;
	float GetGeneratorValue() const;
	float GetMixValue() const;
	float GetValue() const;
	bool IsBypassed() const;
	void MixIn(const float InValue);
	void MixGenerators();
	void Reset();

private:
	void Init(FAtomModulationControlBusSettings&& InSettings);
	float Mix(float ValueA) const;

	float DefaultValue;

	// Cached values
	float GeneratorValue;
	float MixValue;

	bool bBypass;

	FAtomModulationMixFunction MixFunction;
	TArray<FAtomModulationGeneratorHandle> GeneratorHandles;
};

using FAtomModulationBusProxyMap = TMap<FAtomModulationBusId, FAtomModulationControlBusProxy>;
using FAtomModulationBusHandle = TAtomProxyHandle<FAtomModulationBusId, FAtomModulationControlBusProxy, FAtomModulationControlBusSettings>;
