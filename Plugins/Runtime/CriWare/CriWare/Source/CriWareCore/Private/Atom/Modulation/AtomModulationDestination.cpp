
#include "Atom/Modulation/AtomModulationDestination.h"

#include "Containers/Map.h"
#include "Algo/NoneOf.h"
#include "Algo/Transform.h"
#include "Math/TransformCalculus.h"
#include "Async/Async.h"
#include "DSP/FloatArrayMath.h"

#include "Atom/AtomThread.h"
#include "Atom/AtomRuntime.h"
#include "Atom/AtomRuntimeManager.h"
#include "Atom/Modulation/AtomModulationLogs.h"
#include "Atom/Modulation/AtomModulationSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AtomModulationDestination)

FAtomSoundModulationDefaultSettings::FAtomSoundModulationDefaultSettings()
{
	VolumeModulation.Value = 0.0f;
	PitchModulation.Value = 0.0f;
	HighpassModulation.Value = ATOM_MIN_FILTER_FREQUENCY;
	LowpassModulation.Value = ATOM_MAX_FILTER_FREQUENCY;
}

FAtomSoundModulationRoutingSettings::FAtomSoundModulationRoutingSettings()
	: FAtomSoundModulationDefaultSettings()
{
}

FAtomModulationDestination::FAtomModulationDestination(const FAtomModulationDestination& InModulationDestination)
	: RuntimeID(InModulationDestination.RuntimeID)
	, ValueTarget(InModulationDestination.ValueTarget)
	, bIsBuffered(InModulationDestination.bIsBuffered)
	, bValueNormalized(InModulationDestination.bValueNormalized)
	, OutputBuffer(InModulationDestination.OutputBuffer)
	, Parameter(InModulationDestination.Parameter)
{
	FScopeLock OtherLock(&InModulationDestination.HandleCritSection);
	Handles = InModulationDestination.Handles;
}

FAtomModulationDestination::FAtomModulationDestination(FAtomModulationDestination&& InModulationDestination)
	: RuntimeID(MoveTemp(InModulationDestination.RuntimeID))
	, ValueTarget(MoveTemp(InModulationDestination.ValueTarget))
	, bIsBuffered(MoveTemp(InModulationDestination.bIsBuffered))
	, bValueNormalized(MoveTemp(InModulationDestination.bValueNormalized))
	, OutputBuffer(MoveTemp(InModulationDestination.OutputBuffer))
	, Parameter(MoveTemp(InModulationDestination.Parameter))
{
	FScopeLock OtherLock(&InModulationDestination.HandleCritSection);
	Handles = MoveTemp(InModulationDestination.Handles);
}

FAtomModulationDestination& FAtomModulationDestination::operator=(const FAtomModulationDestination& InModulationDestination)
{
	RuntimeID = InModulationDestination.RuntimeID;
	ValueTarget = InModulationDestination.ValueTarget;
	bIsBuffered = InModulationDestination.bIsBuffered;
	bValueNormalized = InModulationDestination.bValueNormalized;
	OutputBuffer = InModulationDestination.OutputBuffer;

	{
		FScopeLock OtherLock(&InModulationDestination.HandleCritSection);
		FScopeLock Lock(&HandleCritSection);
		Handles = InModulationDestination.Handles;
	}

	Parameter = InModulationDestination.Parameter;

	return *this;
}

FAtomModulationDestination& FAtomModulationDestination::operator=(FAtomModulationDestination&& InModulationDestination)
{
	RuntimeID = MoveTemp(InModulationDestination.RuntimeID);
	ValueTarget = MoveTemp(InModulationDestination.ValueTarget);
	bIsBuffered = MoveTemp(InModulationDestination.bIsBuffered);
	bValueNormalized = MoveTemp(InModulationDestination.bValueNormalized);
	bHasProcessed = MoveTemp(InModulationDestination.bHasProcessed);
	OutputBuffer = MoveTemp(InModulationDestination.OutputBuffer);
	{
		FScopeLock OtherLock(&InModulationDestination.HandleCritSection);
		FScopeLock Lock(&HandleCritSection);
		Handles = MoveTemp(InModulationDestination.Handles);
	}

	Parameter = MoveTemp(InModulationDestination.Parameter);

	return *this;
}

void FAtomModulationDestination::ResetHandles()
{
	FAtomModulationParameter ParameterCopy = Parameter;

	FScopeLock Lock(&HandleCritSection);
	Handles.Reset();
	Handles.Add(FAtomModulatorHandle{ MoveTemp(ParameterCopy) });
}

void FAtomModulationDestination::Init(FAtomRuntimeId InRuntimeID, bool bInIsBuffered, bool bInValueNormalized)
{
	Init(InRuntimeID, FName(), bInIsBuffered, bInValueNormalized);
}

void FAtomModulationDestination::Init(FAtomRuntimeId InRuntimeID, FName InParameterName, bool bInIsBuffered, bool bInValueNormalized)
{
	RuntimeID = InRuntimeID;
	bIsBuffered = bInIsBuffered;
	bValueNormalized = bInValueNormalized;

	OutputBuffer.Reset();
	Parameter = Atom::GetModulationParameter(InParameterName);

	ResetHandles();
}

bool FAtomModulationDestination::IsActive()
{
	FScopeLock Lock(&HandleCritSection);
	return Algo::NoneOf(Handles, [](const FAtomModulatorHandle& Handle) { return Handle.IsValid(); });
}

bool FAtomModulationDestination::ProcessControl(float InValueUnitBase, int32 InNumSamples)
{
	bHasProcessed = true;
	float LastTarget = ValueTarget;

	float NewTargetNormalized = Parameter.DefaultValue;
	if (Parameter.bRequiresConversion)
	{
		Parameter.NormalizedFunction(NewTargetNormalized);
	}

	FScopeLock Lock(&HandleCritSection);
	{
		for (const FAtomModulatorHandle& Handle : Handles)
		{
			if (Handle.IsValid())
			{
				float NewHandleValue = 1.0f;
				Handle.GetValue(NewHandleValue);
				Parameter.MixFunction(NewTargetNormalized, NewHandleValue);
			}
		}
	}

	// Convert base to linear space
	float InValueBaseNormalized = InValueUnitBase;
	if (Parameter.bRequiresConversion)
	{
		Parameter.NormalizedFunction(InValueBaseNormalized);
	}

	// Mix in base value
	Parameter.MixFunction(NewTargetNormalized, InValueBaseNormalized);
	ValueTarget = NewTargetNormalized;

	// Convert target to unit space if required
	if (Parameter.bRequiresConversion && !bValueNormalized)
	{
		Parameter.UnitFunction(ValueTarget);
	}

	if (bIsBuffered)
	{
		if (OutputBuffer.Num() != InNumSamples)
		{
			OutputBuffer.Reset();
			OutputBuffer.AddZeroed(InNumSamples);
		}
	}

	// Fade from last target to new if output buffer is active
	if (!OutputBuffer.IsEmpty())
	{
		if (OutputBuffer.Num() % AUDIO_NUM_FLOATS_PER_VECTOR_REGISTER == 0)
		{
			if (FMath::IsNearlyEqual(LastTarget, ValueTarget))
			{
				Audio::ArraySetToConstantInplace(OutputBuffer, ValueTarget);
			}
			else
			{
				Audio::ArraySetToConstantInplace(OutputBuffer, 1.0f);
				Audio::ArrayFade(OutputBuffer, LastTarget, ValueTarget);
			}
		}
		else
		{
			if (FMath::IsNearlyEqual(LastTarget, ValueTarget))
			{
				OutputBuffer.Init(ValueTarget, InNumSamples);
			}
			else
			{
				float SampleValue = LastTarget;
				const float DeltaValue = (ValueTarget - LastTarget) / OutputBuffer.Num();
				for (int32 i = 0; i < OutputBuffer.Num(); ++i)
				{
					OutputBuffer[i] = SampleValue;
					SampleValue += DeltaValue;
				}
			}
		}
	}

	return !FMath::IsNearlyEqual(LastTarget, ValueTarget);
}

void FAtomModulationDestination::UpdateModulators(const TSet<TObjectPtr<UAtomModulatorBase>>& InModulators)
{
	TArray<TUniquePtr<IAtomModulatorSettings>> ProxySettings;
	Algo::TransformIf(
		InModulators,
		ProxySettings,
		[](const UAtomModulatorBase* Mod) { return Mod != nullptr; },
		[](const UAtomModulatorBase* Mod) { return Mod->CreateProxySettings(); }
	);

	UpdateModulatorsInternal(MoveTemp(ProxySettings));
}

void FAtomModulationDestination::UpdateModulators(const TSet<UAtomModulatorBase*>& InModulators)
{
	TArray<TUniquePtr<IAtomModulatorSettings>> ProxySettings;
	Algo::TransformIf(
		InModulators,
		ProxySettings,
		[](const UAtomModulatorBase* Mod) { return Mod != nullptr; },
		[](const UAtomModulatorBase* Mod) { return Mod->CreateProxySettings(); }
	);

	UpdateModulatorsInternal(MoveTemp(ProxySettings));
}

void FAtomModulationDestination::UpdateModulators(const TSet<const UAtomModulatorBase*>& InModulators)
{
	TArray<TUniquePtr<IAtomModulatorSettings>> ProxySettings;
	Algo::TransformIf(
		InModulators,
		ProxySettings,
		[](const UAtomModulatorBase* Mod) { return Mod != nullptr; },
		[](const UAtomModulatorBase* Mod) { return Mod->CreateProxySettings(); }
	);

	UpdateModulatorsInternal(MoveTemp(ProxySettings));
}

void FAtomModulationDestination::UpdateModulatorsInternal(TArray<TUniquePtr<IAtomModulatorSettings>>&& ProxySettings)
{
	auto UpdateHandleLambda = [this, ModSettings = MoveTemp(ProxySettings)]() mutable
	{
		if (FAtomRuntime* AtomRuntime = FAtomRuntimeManager::Get()->GetAtomRuntimeRaw(RuntimeID))
		{
			//if (AudioDevice->IsModulationPluginEnabled() && AudioDevice->ModulationInterface.IsValid())
			//{
			//	if (IAudioModulationManager* Modulation = AudioDevice->ModulationInterface.Get())
				if (FAtomModulationSystem* Modulation = AtomRuntime->GetAtomModulationSystem())
				{
					TSet<FAtomModulatorHandle> NewHandles;
					for (TUniquePtr<IAtomModulatorSettings>& ModSetting : ModSettings)
					{
						FAtomModulationParameter HandleParam = Parameter;
						NewHandles.Add(FAtomModulatorHandle{ *Modulation, *ModSetting.Get(), MoveTemp(HandleParam) });
					}

					FScopeLock Lock(&HandleCritSection);
					Handles = MoveTemp(NewHandles);
				}
				return;
			//}
		}

		ResetHandles();
	};

	FAtomThread::RunCommandOnAtomThread(MoveTemp(UpdateHandleLambda));
}
