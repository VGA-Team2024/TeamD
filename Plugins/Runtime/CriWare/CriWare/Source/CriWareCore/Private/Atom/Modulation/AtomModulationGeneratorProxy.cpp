
#include "AtomModulationGeneratorProxy.h"

#include "Atom/Modulation/AtomModulation.h"
#include "Atom/Modulation/AtomModulationSystem.h"

const FAtomModulationGeneratorId InvalidGeneratorId = INDEX_NONE;

const FAtomModulationParameter& FAtomModulationGeneratorSettings::GetOutputParameter() const
{
	// For now, all generators are normalized values (0, 1).
	return Atom::GetModulationParameter({ });
}

FAtomModulatorTypeId FAtomModulationGeneratorSettings::Register(FAtomModulatorHandleId HandleId, FAtomModulationSystem& InModSystem) const
{
	return InModSystem.RegisterModulator(HandleId, *this);
}

FAtomModulatorGeneratorProxy::FAtomModulatorGeneratorProxy(FAtomModulationGeneratorSettings&& InSettings, FAtomModulationSystem& InModSystem)
	: TAtomModulatorProxyRefType(InSettings.GetName(), InSettings.GetId(), InModSystem)
	, Generator(MoveTemp(InSettings.Generator))
{
	Generator->Init(InModSystem.GetAtomRuntimeId());
}

FAtomModulatorGeneratorProxy& FAtomModulatorGeneratorProxy::operator =(FAtomModulationGeneratorSettings&& InSettings)
{
	if (ensure(InSettings.Generator.IsValid()))
	{
		Generator->UpdateGenerator(MoveTemp(InSettings.Generator));
	}
	else
	{
		Generator.Reset();
	}

	return *this;
}