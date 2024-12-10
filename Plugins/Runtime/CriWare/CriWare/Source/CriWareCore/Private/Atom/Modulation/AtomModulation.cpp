
#include "Atom/Modulation/AtomModulation.h"

#include "Containers/Map.h"
#include "UObject/Object.h"

#include "Atom/AtomRuntime.h"
#include "Atom/AtomRuntimeManager.h"
#include "Atom/Modulation/AtomModulationLogs.h"
#include "Atom/Modulation/AtomModulationSystem.h"

DEFINE_LOG_CATEGORY(LogAtomModulation);

#include UE_INLINE_GENERATED_CPP_BY_NAME(AtomModulation)

namespace AtomModulationInterfacePrivate
{
	class FAtomModulationParameterRegistry
	{
		TMap<FName, FAtomModulationParameter> Values;

		mutable FCriticalSection ThreadSafeValueAccessor;

	public:
		bool IsRegistered(FName InName) const
		{
			FScopeLock Lock(&ThreadSafeValueAccessor);
			return Values.Contains(InName);
		}

		void Register(FName InName, FAtomModulationParameter&& InParameter)
		{
			FScopeLock Lock(&ThreadSafeValueAccessor);
			if (FAtomModulationParameter* Value = Values.Find(InName))
			{
				*Value = MoveTemp(InParameter);
			}
			else
			{
				Values.Add(InName, MoveTemp(InParameter));
			}
		}

		bool Unregister(FName InName)
		{
			FScopeLock Lock(&ThreadSafeValueAccessor);
			return Values.Remove(InName) > 0;
		}

		void UnregisterAll()
		{
			FScopeLock Lock(&ThreadSafeValueAccessor);
			Values.Reset();
		}

		const FAtomModulationParameter& Get(FName InName) const
		{
			FScopeLock Lock(&ThreadSafeValueAccessor);

			static const FAtomModulationParameter DefaultParameter{ };
			if (const FAtomModulationParameter* Param = Values.Find(InName))
			{
				return *Param;
			}

			return DefaultParameter;
		}
	} ParameterRegistry;
} // namespace

FAtomModulatorHandleId CreateModulatorHandleId()
{
	static FAtomModulatorHandleId NextHandleId = INDEX_NONE;
	return ++NextHandleId;
}

FAtomModulationParameter::FAtomModulationParameter()
	: MixFunction(GetDefaultMixFunction())
	, UnitFunction(GetDefaultUnitConversionFunction())
	, NormalizedFunction(GetDefaultNormalizedConversionFunction())
{
}

FAtomModulationParameter::FAtomModulationParameter(FAtomModulationParameter&& InParam)
	: ParameterName(MoveTemp(InParam.ParameterName))
	, DefaultValue(InParam.DefaultValue)
	, MinValue(InParam.MinValue)
	, MaxValue(InParam.MaxValue)
	, bRequiresConversion(InParam.bRequiresConversion)
#if WITH_EDITORONLY_DATA
	, UnitDisplayName(MoveTemp(InParam.UnitDisplayName))
	, ClassName(MoveTemp(InParam.ClassName))
#endif // WITH_EDITORONLY_DATA
	, MixFunction(MoveTemp(InParam.MixFunction))
	, UnitFunction(MoveTemp(InParam.UnitFunction))
	, NormalizedFunction(MoveTemp(InParam.NormalizedFunction))
{
}

FAtomModulationParameter::FAtomModulationParameter(const FAtomModulationParameter& InParam)
	: ParameterName(InParam.ParameterName)
	, DefaultValue(InParam.DefaultValue)
	, MinValue(InParam.MinValue)
	, MaxValue(InParam.MaxValue)
	, bRequiresConversion(InParam.bRequiresConversion)
#if WITH_EDITORONLY_DATA
	, UnitDisplayName(InParam.UnitDisplayName)
	, ClassName(InParam.ClassName)
#endif // WITH_EDITORONLY_DATA
	, MixFunction(InParam.MixFunction)
	, UnitFunction(InParam.UnitFunction)
	, NormalizedFunction(InParam.NormalizedFunction)
{
}

FAtomModulationParameter& FAtomModulationParameter::operator=(const FAtomModulationParameter& InParam)
{
	ParameterName = InParam.ParameterName;
	DefaultValue = InParam.DefaultValue;
	MinValue = InParam.MinValue;
	MaxValue = InParam.MaxValue;
	bRequiresConversion = InParam.bRequiresConversion;

#if WITH_EDITORONLY_DATA
	UnitDisplayName = InParam.UnitDisplayName;
	ClassName = InParam.ClassName;
#endif // WITH_EDITORONLY_DATA

	MixFunction = InParam.MixFunction;
	UnitFunction = InParam.UnitFunction;
	NormalizedFunction = InParam.NormalizedFunction;

	return *this;
}

FAtomModulationParameter& FAtomModulationParameter::operator=(FAtomModulationParameter&& InParam)
{
	ParameterName = MoveTemp(InParam.ParameterName);
	DefaultValue = InParam.DefaultValue;
	MinValue = InParam.MinValue;
	MaxValue = InParam.MaxValue;
	bRequiresConversion = InParam.bRequiresConversion;

#if WITH_EDITORONLY_DATA
	UnitDisplayName = MoveTemp(InParam.UnitDisplayName);
	ClassName = MoveTemp(InParam.ClassName);
#endif // WITH_EDITORONLY_DATA

	MixFunction = MoveTemp(InParam.MixFunction);
	UnitFunction = MoveTemp(InParam.UnitFunction);
	NormalizedFunction = MoveTemp(InParam.NormalizedFunction);

	return *this;
}

const FAtomModulationMixFunction& FAtomModulationParameter::GetDefaultMixFunction()
{
	static const FAtomModulationMixFunction DefaultMixFunction = [](float& InOutValueA, float InValueB)
	{
		InOutValueA *= InValueB;
	};

	return DefaultMixFunction;
}

const FAtomModulationUnitConversionFunction& FAtomModulationParameter::GetDefaultUnitConversionFunction()
{
	static const FAtomModulationUnitConversionFunction ConversionFunction = [](float& InOutValue)
	{
	};

	return ConversionFunction;
};

const FAtomModulationNormalizedConversionFunction& FAtomModulationParameter::GetDefaultNormalizedConversionFunction()
{
	static const FAtomModulationNormalizedConversionFunction ConversionFunction = [](float& InOutValue)
	{
	};

	return ConversionFunction;
};

namespace Atom
{
	void RegisterModulationParameter(FName InName, FAtomModulationParameter&& InParameter)
	{
		using namespace AtomModulationInterfacePrivate;
		ParameterRegistry.Register(InName, MoveTemp(InParameter));
	}

	bool UnregisterModulationParameter(FName InName)
	{
		using namespace AtomModulationInterfacePrivate;
		return ParameterRegistry.Unregister(InName);
	}

	void UnregisterAllModulationParameters()
	{
		using namespace AtomModulationInterfacePrivate;
		ParameterRegistry.UnregisterAll();
	}

	bool IsModulationParameterRegistered(FName InName)
	{
		using namespace AtomModulationInterfacePrivate;
		return ParameterRegistry.IsRegistered(InName);
	}

	const FAtomModulationParameter& GetModulationParameter(FName InName)
	{
		using namespace AtomModulationInterfacePrivate;
		return ParameterRegistry.Get(InName);
	}

	void IterateOverAllModulationSystems(TFunctionRef<void(FAtomModulationSystem&)> InFunction)
	{
		if (FAtomRuntimeManager* RuntimeManager = FAtomRuntimeManager::Get())
		{
			RuntimeManager->IterateOverAllRuntimes([ModFunction = MoveTemp(InFunction)](FAtomRuntimeId RuntimeId, FAtomRuntime* AtomRuntime)
			{
				if (AtomRuntime)// && AtomRuntime->IsModulationPluginEnabled() && AudioDevice->ModulationInterface.IsValid())
				{
					//auto ModulationInterface = static_cast<AudioModulation::FAudioModulationManager*>(AudioDevice->ModulationInterface.Get());
					auto Modulation = AtomRuntime->GetAtomModulationSystem();
					ModFunction(*Modulation);
				}
			});
		}
	}
} // namespace

/*
 * FAtomModulatorHandle
 *****************************************************************************/

FAtomModulatorHandle::FAtomModulatorHandle(FAtomModulationParameter&& InParameter)
	: Parameter(InParameter)
	, HandleId(CreateModulatorHandleId())
{
}

FAtomModulatorHandle::FAtomModulatorHandle(FAtomModulationSystem& InModulation, const IAtomModulatorSettings& InModulatorSettings, FAtomModulationParameter&& InParameter)
	: Parameter(MoveTemp(InParameter))
	, HandleId(CreateModulatorHandleId())
	, Modulation(InModulation.AsShared())
{
	ModulatorTypeId = InModulatorSettings.Register(HandleId, InModulation);
	if (ModulatorTypeId != INDEX_NONE)
	{
		ModulatorId = InModulatorSettings.GetModulatorId();
	}
}

FAtomModulatorHandle::FAtomModulatorHandle(const FAtomModulatorHandle& InOther)
{
	HandleId = CreateModulatorHandleId();

	if (TSharedPtr<FAtomModulationSystem> ModPtr = InOther.Modulation.Pin())
	{
		ModPtr->RegisterModulator(HandleId, InOther.ModulatorId);
		Parameter = InOther.Parameter;
		ModulatorId = InOther.ModulatorId;
		ModulatorTypeId = InOther.ModulatorTypeId;
		Modulation = InOther.Modulation;
	}
}

FAtomModulatorHandle::FAtomModulatorHandle(FAtomModulatorHandle&& InOther)
	: Parameter(MoveTemp(InOther.Parameter))
	, HandleId(InOther.HandleId)
	, ModulatorTypeId(InOther.ModulatorTypeId)
	, ModulatorId(InOther.ModulatorId)
	, Modulation(InOther.Modulation)
{
	// Move does not register as presumed already activated or
	// copying default handle, which is invalid. Removes data
	// from handle being moved to avoid double deactivation on
	// destruction.
	InOther.Parameter = FAtomModulationParameter();
	InOther.HandleId = INDEX_NONE;
	InOther.ModulatorTypeId = INDEX_NONE;
	InOther.ModulatorId = INDEX_NONE;
	InOther.Modulation.Reset();
}

FAtomModulatorHandle::~FAtomModulatorHandle()
{
	if (TSharedPtr<FAtomModulationSystem> ModPtr = Modulation.Pin())
	{
		ModPtr->UnregisterModulator(*this);
	}
}

FAtomModulatorHandle& FAtomModulatorHandle::operator=(const FAtomModulatorHandle& InOther)
{
	Parameter = InOther.Parameter;

	if (TSharedPtr<FAtomModulationSystem> ModPtr = InOther.Modulation.Pin())
	{
		HandleId = CreateModulatorHandleId();
		ModulatorId = InOther.ModulatorId;
		ModulatorTypeId = InOther.ModulatorTypeId;
		Modulation = InOther.Modulation;

		if (ModulatorId != INDEX_NONE)
		{
			ModPtr->RegisterModulator(HandleId, ModulatorId);
		}
	}
	else
	{
		HandleId = INDEX_NONE;
		ModulatorId = INDEX_NONE;
		ModulatorTypeId = INDEX_NONE;
		Modulation.Reset();
	}

	return *this;
}

FAtomModulatorHandle& FAtomModulatorHandle::operator=(FAtomModulatorHandle&& InOther)
{
	if (HandleId != INDEX_NONE)
	{
		if (TSharedPtr<FAtomModulationSystem> ModPtr = Modulation.Pin())
		{
			ModPtr->UnregisterModulator(*this);
		}
	}

	// Move does not activate as presumed already activated or
	// copying default handle, which is invalid. Removes data
	// from handle being moved to avoid double deactivation on
	// destruction.
	Parameter = MoveTemp(InOther.Parameter);
	HandleId = InOther.HandleId;
	ModulatorId = InOther.ModulatorId;
	ModulatorTypeId = InOther.ModulatorTypeId;
	Modulation = InOther.Modulation;

	InOther.Parameter = FAtomModulationParameter();
	InOther.HandleId = INDEX_NONE;
	InOther.ModulatorId = INDEX_NONE;
	InOther.ModulatorTypeId = INDEX_NONE;
	InOther.Modulation.Reset();

	return *this;
}

FAtomModulatorId FAtomModulatorHandle::GetModulatorId() const
{
	return ModulatorId;
}

const FAtomModulationParameter& FAtomModulatorHandle::GetParameter() const
{
	return Parameter;
}

FAtomModulatorTypeId FAtomModulatorHandle::GetTypeId() const
{
	return ModulatorTypeId;
}

uint32 FAtomModulatorHandle::GetHandleId() const
{
	return HandleId;
}

bool FAtomModulatorHandle::GetValue(float& OutValue) const
{
	check(IsValid());

	OutValue = 1.0f;

	if (TSharedPtr<FAtomModulationSystem> ModPtr = Modulation.Pin())
	{
		return ModPtr->GetModulatorValue(*this, OutValue);
	}

	return false;
}

bool FAtomModulatorHandle::GetValueThreadSafe(float& OutValue) const
{
	check(IsValid());

	OutValue = 1.0f;
	if (TSharedPtr<FAtomModulationSystem> ModPtr = Modulation.Pin())
	{
		return ModPtr->GetModulatorValueThreadSafe(*this, OutValue);
	}

	return false;
}

bool FAtomModulatorHandle::IsValid() const
{
	return ModulatorId != INDEX_NONE;
}


const FAtomModulationParameter& UAtomModulatorBase::GetOutputParameter() const
{
	return Atom::GetModulationParameter({ });
}

//TUniquePtr<Audio::IProxyData> USoundModulatorBase::CreateNewProxyData(const Audio::FProxyDataInitParams& InitParams)
//{
//	// This should never be hit as all instances of modulators should implement their own version of the proxy data interface.
//	checkNoEntry();
//	return TUniquePtr<Audio::IProxyData>();
//}

TUniquePtr<IAtomModulatorSettings> UAtomModulatorBase::CreateProxySettings() const
{
	checkNoEntry();
	return TUniquePtr<IAtomModulatorSettings>();
}
