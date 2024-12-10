
#include "Atom/Modulation/AtomModulationControlBus.h"

#include "Engine/World.h"

#include "Atom/AtomRuntime.h"
#include "Atom/AtomRuntimeManager.h"
#include "Atom/Modulation/AtomModulation.h"
#include "Atom/Modulation/AtomModulationLogs.h"
//#include "Atom/Modulation/AtomModulationStatics.h"
#include "Atom/Modulation/AtomModulationSystem.h"
#include "Atom/Modulation/AtomModulationControlBusProxy.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AtomModulationControlBus)

UAtomModulationControlBus::UAtomModulationControlBus(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bBypass(false)
#if WITH_EDITORONLY_DATA
	, bOverrideAddress(false)
#endif // WITH_EDITORONLY_DATA
	, Parameter(nullptr)
{
}

TUniquePtr<IAtomModulatorSettings> UAtomModulationControlBus::CreateProxySettings() const
{
	return TUniquePtr<IAtomModulatorSettings>(new FAtomModulationControlBusSettings(*this));
}

#if WITH_EDITOR
void UAtomModulationControlBus::PostDuplicate(EDuplicateMode::Type DuplicateMode)
{
	if (!bOverrideAddress)
	{
		Address = GetName();
	}

	Super::PostDuplicate(DuplicateMode);
}

void UAtomModulationControlBus::PostEditChangeProperty(FPropertyChangedEvent& InPropertyChangedEvent)
{
	if (FProperty* Property = InPropertyChangedEvent.Property)
	{
		if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAtomModulationControlBus, bOverrideAddress) && !bOverrideAddress)
		{
			Address = GetName();
		}

		if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAtomModulationControlBus, Parameter))
		{
			/*for (TObjectIterator<UAtomModulationControlBusMix> Iter; Iter; ++Iter)
			{
				if (UAtomModulationControlBusMix* Mix = *Iter)
				{
					for (FAtomModulationControlBusMixStage& Stage : Mix->MixStages)
					{
						if (Stage.Bus == this)
						{
							float UnitValue = Stage.Value.TargetValue;
							if (Parameter)
							{
								UnitValue = Parameter->ConvertNormalizedToUnit(Stage.Value.TargetValue);
							}

							if (!FMath::IsNearlyEqual(Stage.Value.TargetUnitValue, UnitValue, KINDA_SMALL_NUMBER))
							{
								Stage.Value.TargetUnitValue = UnitValue;
							}
						}
					}
				}
			}*/
		}

		Atom::IterateOverAllModulationSystems([this](FAtomModulationSystem& OutModSystem)
		{
			OutModSystem.UpdateModulator(*this);
		});
	}

	Super::PostEditChangeProperty(InPropertyChangedEvent);
}

void UAtomModulationControlBus::PostInitProperties()
{
	if (!bOverrideAddress)
	{
		Address = GetName();
	}

	Super::PostInitProperties();
}

void UAtomModulationControlBus::PostRename(UObject* OldOuter, const FName OldName)
{
	if (!bOverrideAddress)
	{
		Address = GetName();
	}
}
#endif // WITH_EDITOR

void UAtomModulationControlBus::BeginDestroy()
{
	if (UWorld* World = GetWorld())
	{
		FAtomRuntimeHandle AtomRuntime = FAtomRuntimeManager::GetAtomRuntimeFromWorld(World);
		if (AtomRuntime.IsValid())
		{
			if (FAtomModulationSystem* Modulation = AtomRuntime->GetAtomModulationSystem())
			{
				check(Modulation);
				Modulation->DeactivateBus(*this);
			}
		}
	}

	// Call parent destroy at end to ensure object is in a valid state for the modulation manager to clean up first
	Super::BeginDestroy();
}

const FAtomModulationMixFunction UAtomModulationControlBus::GetMixFunction() const
{
	if (Parameter)
	{
		return Parameter->GetMixFunction();
	}

	return FAtomModulationParameter::GetDefaultMixFunction();
}

//TUniquePtr<Audio::IProxyData> UAtomModulationControlBus::CreateNewProxyData(const Audio::FProxyDataInitParams& InitParams)
//{
//	return MakeUnique<FAtomModulatorAssetProxy>(*this);
//}

const FAtomModulationParameter& UAtomModulationControlBus::GetOutputParameter() const
{
	const FString Breadcrumb = FString::Format(TEXT("{0} '{1}'"), { *GetClass()->GetName(), *GetName() });
	return Atom::GetOrRegisterParameter(Parameter, Breadcrumb);
}

