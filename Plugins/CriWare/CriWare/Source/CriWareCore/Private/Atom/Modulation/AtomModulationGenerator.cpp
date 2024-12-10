
#include "Atom/Modulation/AtomModulationGenerator.h"

#include "Engine/World.h"
#include "Templates/Function.h"

#include "Atom/AtomRuntime.h"
#include "Atom/Modulation/AtomModulation.h"
#include "Atom/Modulation/AtomModulationLogs.h"
#include "Atom/Modulation/AtomModulationSystem.h"
#include "Atom/Modulation/AtomModulationGeneratorProxy.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AtomModulationGenerator)

void IAtomModulatorGenerator::AtomThreadCommand(TUniqueFunction<void()>&& InCommand)
{
	CommandQueue.Enqueue(MoveTemp(InCommand));
}

void IAtomModulatorGenerator::PumpCommands()
{
	TUniqueFunction<void()> Cmd;
	while (!CommandQueue.IsEmpty())
	{
		CommandQueue.Dequeue(Cmd);
		Cmd();
	}
}

#if WITH_EDITOR
void UAtomModulationGenerator::PostEditChangeProperty(FPropertyChangedEvent& InPropertyChangedEvent)
{
	// Guards against slamming the modulation system with changes when using sliders.
	if (InPropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		Atom::IterateOverAllModulationSystems([this](FAtomModulationSystem& OutModulation)
		{
			OutModulation.UpdateModulator(*this);
		});
	}

	Super::PostEditChangeProperty(InPropertyChangedEvent);
}
#endif // WITH_EDITOR

//TUniquePtr<IAtomProxyData> UAtomModulationGenerator::CreateNewProxyData(const Audio::FProxyDataInitParams& InitParams)
//{
//	return MakeUnique<FAtomModulatorAssetProxy>(*this);
//}

TUniquePtr<IAtomModulatorSettings> UAtomModulationGenerator::CreateProxySettings() const
{
	return TUniquePtr<IAtomModulatorSettings>(new FAtomModulationGeneratorSettings(*this));
}

void UAtomModulationGenerator::BeginDestroy()
{
	if (UWorld* World = GetWorld())
	{
		FAtomRuntimeHandle AtomRuntime = FAtomRuntimeManager::GetAtomRuntimeFromWorld(World);
		if (AtomRuntime.IsValid())
		{	
			if (FAtomModulationSystem* Modulation = AtomRuntime->GetAtomModulationSystem())
			{
				check(Modulation);
				Modulation->DeactivateGenerator(*this);
			}
		}
	}

	Super::BeginDestroy();
}
