
#include "AtomModulationControlBusProxy.h"

#include "Engine/World.h"

#include "Atom/AtomRuntime.h"
#include "Atom/AtomRuntimeManager.h"
#include "Atom/Modulation/AtomModulation.h"
#include "Atom/Modulation/AtomModulationLogs.h"
#include "Atom/Modulation/AtomModulationSystem.h"
#include "Atom/Modulation/AtomModulationGenerator.h"

const FAtomModulationBusId InvalidBusId = INDEX_NONE;

FAtomModulatorTypeId FAtomModulationControlBusSettings::Register(FAtomModulatorHandleId HandleId, FAtomModulationSystem& InModulation) const
{
	return InModulation.RegisterModulator(HandleId, *this);
}

FAtomModulationControlBusProxy::FAtomModulationControlBusProxy()
	: DefaultValue(0.0f)
	, GeneratorValue(1.0f)
	, MixValue(NAN)
	, bBypass(false)
{
}

FAtomModulationControlBusProxy::FAtomModulationControlBusProxy(FAtomModulationControlBusSettings&& InSettings, FAtomModulationSystem& InModSystem)
	: TAtomModulatorProxyRefType(InSettings.GetName(), InSettings.GetId(), InModSystem)
{
	Init(MoveTemp(InSettings));
}

FAtomModulationControlBusProxy& FAtomModulationControlBusProxy::operator =(FAtomModulationControlBusSettings&& InSettings)
{
	Init(MoveTemp(InSettings));
	return *this;
}

float FAtomModulationControlBusProxy::GetDefaultValue() const
{
	return DefaultValue;
}

const TArray<FAtomModulationGeneratorHandle>& FAtomModulationControlBusProxy::GetGeneratorHandles() const
{
	return GeneratorHandles;
}

float FAtomModulationControlBusProxy::GetGeneratorValue() const
{
	return GeneratorValue;
}

float FAtomModulationControlBusProxy::GetMixValue() const
{
	return MixValue;
}

float FAtomModulationControlBusProxy::GetValue() const
{
	const float DefaultMixed = Mix(DefaultValue);
	return FMath::Clamp(DefaultMixed * GeneratorValue, 0.0f, 1.0f);
}

void FAtomModulationControlBusProxy::Init(FAtomModulationControlBusSettings&& InSettings)
{
	check(ModSystem);

	GeneratorValue = 1.0f;
	MixValue = NAN;
	MixFunction = MoveTemp(InSettings.MixFunction);

	DefaultValue = FMath::Clamp(InSettings.DefaultValue, 0.0f, 1.0f);
	bBypass = InSettings.bBypass;

	TArray<FAtomModulationGeneratorHandle> NewHandles;
	for (FAtomModulationGeneratorSettings& GeneratorSettings : InSettings.GeneratorSettings)
	{
		NewHandles.Add(FAtomModulationGeneratorHandle::Create(MoveTemp(GeneratorSettings), ModSystem->RefProxies.Generators, *ModSystem));
	}

	// Move vs. reset and adding to original array to avoid potentially clearing handles (and thus current Generator state)
	// and destroying generators if function is called while reinitializing/updating the modulator
	GeneratorHandles = MoveTemp(NewHandles);
}

bool FAtomModulationControlBusProxy::IsBypassed() const
{
	return bBypass;
}

float FAtomModulationControlBusProxy::Mix(float ValueA) const
{
	// If mix value is NaN, it is uninitialized (effectively, the parent bus is inactive)
	// and therefore not mixable, so just return the second value.
	if (FMath::IsNaN(MixValue))
	{
		return ValueA;
	}

	float OutValue = MixValue;
	MixFunction(OutValue, ValueA);
	return OutValue;
}

void FAtomModulationControlBusProxy::MixIn(const float InValue)
{
	MixValue = Mix(InValue);
}

void FAtomModulationControlBusProxy::MixGenerators()
{
	for (const FAtomModulationGeneratorHandle& Handle : GeneratorHandles)
	{
		if (Handle.IsValid())
		{
			const FAtomModulatorGeneratorProxy& GeneratorProxy = Handle.FindProxy();
			if (!GeneratorProxy.IsBypassed())
			{
				GeneratorValue *= GeneratorProxy.GetValue();
			}
		}
	}
}

void FAtomModulationControlBusProxy::Reset()
{
	GeneratorValue = 1.0f;
	MixValue = NAN;
}
