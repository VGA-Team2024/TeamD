
#include "AtomModulationControlBusMixProxy.h"

#include "Audio/AudioAddressPattern.h"

#include "Atom/AtomRuntime.h"
#include "Atom/AtomRuntimeManager.h"
#include "Atom/Modulation/AtomModulation.h"
#include "Atom/Modulation/AtomModulationLogs.h"
//#include "Atom/Modulation/AtomModulationProfileSerializer.h"
#include "Atom/Modulation/AtomModulationStatics.h"
#include "Atom/Modulation/AtomModulationSystem.h"
#include "Atom/Modulation/AtomModulationControlBus.h"
#include "Atom/Modulation/AtomModulationControlBusMix.h"

#define LOCTEXT_NAMESPACE "AtomModulation"


const FAtomModulationBusMixId InvalidBusMixId = INDEX_NONE;

FAtomModulatorBusMixStageSettings::FAtomModulatorBusMixStageSettings(const FAtomModulationControlBusMixStage& InStage)
	: TAtomModulatorBase<FAtomModulationBusId>(InStage.Bus->GetName(), InStage.Bus->GetUniqueID())
	, Address(InStage.Bus->Address)
	, ParamClassId(INDEX_NONE)
	, ParamId(INDEX_NONE)
	, Value(InStage.Value)
	, BusSettings(FAtomModulationControlBusSettings(*InStage.Bus))
{
	if (UAtomModulationParameter* Parameter = InStage.Bus->Parameter)
	{
		ParamId = Parameter->GetUniqueID();

		UClass* Class = Parameter->GetClass();
		check(Class);
		ParamClassId = Class->GetUniqueID();

		Value.SetCurrentValue(Parameter->Settings.ValueNormalized);
	}
}

FAtomModulatorBusMixStageProxy::FAtomModulatorBusMixStageProxy(FAtomModulatorBusMixStageSettings&& InSettings, FAtomModulationSystem& OutModSystem)
	: TAtomModulatorBase<FAtomModulationBusId>(InSettings.BusSettings.GetName(), InSettings.BusSettings.GetId())
	, Address(MoveTemp(InSettings.Address))
	, ParamClassId(InSettings.ParamClassId)
	, ParamId(InSettings.ParamId)
	, Value(InSettings.Value)
	, BusHandle(FAtomModulationBusHandle::Create(MoveTemp(InSettings.BusSettings), OutModSystem.RefProxies.Buses, OutModSystem))
{
}

FAtomModulatorBusMixSettings::FAtomModulatorBusMixSettings(const UAtomModulationControlBusMix& InBusMix)
	: TAtomModulatorBase<FAtomModulationBusMixId>(InBusMix.GetName(), InBusMix.GetUniqueID())
{
	for (const FAtomModulationControlBusMixStage& Stage : InBusMix.MixStages)
	{
		if (Stage.Bus)
		{
			Stages.Add(FAtomModulatorBusMixStageSettings(Stage));
		}
		else
		{
			UE_LOG(LogAtomModulation, VeryVerbose,
				TEXT("UAtomModulationControlBusMix '%s' has stage with no bus specified. "
					"Mix instance initialized with stage ignored."),
				*InBusMix.GetFullName());
		}
	}
}

FAtomModulatorBusMixSettings::FAtomModulatorBusMixSettings(FAtomModulatorBusMixSettings&& InBusMixSettings)
	: TAtomModulatorBase<FAtomModulationBusMixId>(InBusMixSettings.GetName(), InBusMixSettings.GetId())
	, Stages(MoveTemp(InBusMixSettings.Stages))
{
}

FAtomModulatorBusMixProxy::FAtomModulatorBusMixProxy(FAtomModulatorBusMixSettings&& InSettings, FAtomModulationSystem& OutModSystem)
	: TAtomModulatorProxyRefType(InSettings.GetName(), InSettings.GetId(), OutModSystem)
{
	SetEnabled(MoveTemp(InSettings));
}

FAtomModulatorBusMixProxy& FAtomModulatorBusMixProxy::operator=(FAtomModulatorBusMixSettings&& InSettings)
{
	SetEnabled(MoveTemp(InSettings));

	return *this;
}

FAtomModulatorBusMixProxy::EStatus FAtomModulatorBusMixProxy::GetStatus() const
{
	return Status;
}

void FAtomModulatorBusMixProxy::Reset()
{
	Stages.Reset();
}

void FAtomModulatorBusMixProxy::SetEnabled(FAtomModulatorBusMixSettings&& InSettings)
{
	check(ModSystem);

	// Cache stages to avoid releasing stage state (and potentially referenced bus state) when re-enabling
	FStageMap CachedStages = Stages;

	Status = EStatus::Enabled;
	Stages.Reset();
	for (FAtomModulatorBusMixStageSettings& StageSettings : InSettings.Stages)
	{
		const FAtomModulationBusId BusId = StageSettings.GetId();
		FAtomModulatorBusMixStageProxy StageProxy(MoveTemp(StageSettings), *ModSystem);
		if (const FAtomModulatorBusMixStageProxy* CachedStage = CachedStages.Find(BusId))
		{
			StageProxy.Value.SetCurrentValue(CachedStage->Value.GetCurrentValue());
		}

		Stages.Add(BusId, MoveTemp(StageProxy));
	}
}

void FAtomModulatorBusMixProxy::SetMix(const TArray<FAtomModulatorBusMixStageSettings>& InStages, float InFadeTime)
{
	for (const FAtomModulatorBusMixStageSettings& NewStage : InStages)
	{
		const FAtomModulationBusId BusId = NewStage.GetId();
		if (FAtomModulatorBusMixStageProxy* StageProxy = Stages.Find(BusId))
		{
			StageProxy->Value.TargetValue = NewStage.Value.TargetValue;
			StageProxy->Value.AttackTime = NewStage.Value.AttackTime;
			StageProxy->Value.ReleaseTime = NewStage.Value.ReleaseTime;

			// Setting entire mix wipes pre-existing user fade requests
			StageProxy->Value.SetActiveFade(FAtomModulationMixValue::EActiveFade::Override, InFadeTime);
		}
	}
}

void FAtomModulatorBusMixProxy::SetMixByFilter(const FString& InAddressFilter, uint32 InParamClassId, uint32 InParamId, float InValue, float InFadeTime)
{
	for (TPair<FAtomModulationBusId, FAtomModulatorBusMixStageProxy>& IdProxyPair : Stages)
	{
		FAtomModulatorBusMixStageProxy& StageProxy = IdProxyPair.Value;
		if (InParamId != INDEX_NONE && StageProxy.ParamId != InParamId)
		{
			continue;
		}

		if (InParamClassId != INDEX_NONE && StageProxy.ParamClassId != InParamClassId)
		{
			continue;
		}

		if (!FAudioAddressPattern::PartsMatch(InAddressFilter, StageProxy.Address))
		{
			continue;
		}

		StageProxy.Value.TargetValue = InValue;
		StageProxy.Value.SetActiveFade(FAtomModulationMixValue::EActiveFade::Override, InFadeTime);
	}
}

void FAtomModulatorBusMixProxy::SetStopping()
{
	if (Status == EStatus::Enabled)
	{
		Status = EStatus::Stopping;
	}
}

void FAtomModulatorBusMixProxy::Update(const double InElapsed, FAtomModulationBusProxyMap& OutProxyMap)
{
	bool bRequestStop = true;
	for (TPair<FAtomModulationBusId, FAtomModulatorBusMixStageProxy>& Stage : Stages)
	{
		FAtomModulatorBusMixStageProxy& StageProxy = Stage.Value;
		FAtomModulationMixValue& MixStageValue = StageProxy.Value;

		if (FAtomModulationControlBusProxy* BusProxy = OutProxyMap.Find(StageProxy.GetId()))
		{
			MixStageValue.Update(InElapsed);

			const float CurrentValue = MixStageValue.GetCurrentValue();
			if (Status == EStatus::Stopping)
			{
				MixStageValue.TargetValue = BusProxy->GetDefaultValue();
				MixStageValue.SetActiveFade(FAtomModulationMixValue::EActiveFade::Release);
				if (!FMath::IsNearlyEqual(MixStageValue.TargetValue, CurrentValue))
				{
					bRequestStop = false;
				}
			}
			else
			{
				bRequestStop = false;
			}
			BusProxy->MixIn(CurrentValue);
		}
	}

	if (bRequestStop)
	{
		Status = EStatus::Stopped;
	}
}

#undef LOCTEXT_NAMESPACE
