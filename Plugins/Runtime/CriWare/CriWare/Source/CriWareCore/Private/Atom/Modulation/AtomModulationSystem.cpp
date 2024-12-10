
#include "Atom/Modulation/AtomModulationSystem.h"

#include "Async/Async.h"
#include "CoreGlobals.h"
#include "Engine/Engine.h"
#include "HAL/PlatformTLS.h"
#include "Misc/CoreDelegates.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "UObject/UObjectIterator.h"
#include "UObject/WeakObjectPtr.h"
#include "Audio/AudioAddressPattern.h"

//#include "AudioModulationProfileSerializer.h"
//#include "AudioModulationSettings.h"
//#include "SoundModulationPatchProxy.h"

#include "Atom/Modulation/AtomModulationLogs.h"
#include "Atom/Modulation/AtomModulationControlBus.h"
#include "Atom/Modulation/AtomModulationGenerator.h"
#include "Atom/Modulation/AtomModulationControlBusMix.h"
#include "AtomModulationControlBusProxy.h"
#include "AtomModulationGeneratorProxy.h"
#include "AtomModulationControlBusMixProxy.h"
#include "AtomModulationProxy.h"

#if !UE_BUILD_SHIPPING
#include "AtomModulationDebug.h"
#endif // !UE_BUILD_SHIPPING

DECLARE_DWORD_COUNTER_STAT(TEXT("Bus Count"), STAT_AtomModulationBusCount, STATGROUP_AtomModulation)
DECLARE_DWORD_COUNTER_STAT(TEXT("Generator Count"), STAT_AtomModulationGeneratorCount, STATGROUP_AtomModulation)
DECLARE_DWORD_COUNTER_STAT(TEXT("Mix Count"),	STAT_AtomModulationMixCount, STATGROUP_AtomModulation)
//DECLARE_DWORD_COUNTER_STAT(TEXT("Patch Count"), STAT_AtomModulationPatchCount, STATGROUP_AtomModulation)
DECLARE_DWORD_COUNTER_STAT(TEXT("Render Queue Commands Processed"), STAT_AtomModulationProcQueueCount, STATGROUP_AtomModulation)

enum class EAtomModulatorType : FAtomModulatorTypeId
{
	Patch,
	Bus,
	Generator,

	Count
};

void FAtomModulationSystem::Initialize(FAtomRuntimeId RumtimeID)
{
#if !UE_BUILD_SHIPPING
	Debugger = MakeShared<AtomModulation::FAtomModulationDebugger>();
#endif // !UE_BUILD_SHIPPING

	AtomRuntimeId = RumtimeID;
}

void FAtomModulationSystem::OnAuditionEnd()
{
	DeactivateAllBusMixes();
}

#if !UE_BUILD_SHIPPING
bool FAtomModulationSystem::OnPostHelp(FCommonViewportClient* ViewportClient, const TCHAR* Stream)
{
	check(IsInGameThread());
	return ViewportClient ? Debugger->OnPostHelp(*ViewportClient, Stream) : true;
}

int32 FAtomModulationSystem::OnRenderStat(UViewportStatsSubsystem* ViewportSubSystem, int32 OffsetX, int32 OffsetY)
{
	check(IsInGameThread());
	return ViewportSubSystem ? Debugger->OnRenderStat(ViewportSubSystem, OffsetX, OffsetY) : OffsetY;
}

int32 FAtomModulationSystem::OnRenderStat(FViewport* Viewport, FCanvas* Canvas, int32 X, int32 Y, const UFont& Font)
{
	check(IsInGameThread());
	return Canvas ? Debugger->OnRenderStat(*Canvas, X, Y, Font) : Y;
}

bool FAtomModulationSystem::OnToggleStat(FCommonViewportClient* ViewportClient, const TCHAR* Stream)
{
	check(IsInGameThread());
	return ViewportClient ? Debugger->OnToggleStat(*ViewportClient, Stream) : true;
}
#endif // !UE_BUILD_SHIPPING

void FAtomModulationSystem::ActivateBus(const UAtomModulationControlBus& InBus)
{
	RunCommandOnProcessingThread([this, Settings = FAtomModulationControlBusSettings(InBus)]() mutable
	{
		FAtomModulationBusHandle BusHandle = FAtomModulationBusHandle::Create(MoveTemp(Settings), RefProxies.Buses, *this);
		ManuallyActivatedBuses.Add(MoveTemp(BusHandle));
	});
}

void FAtomModulationSystem::ActivateBusMix(FAtomModulatorBusMixSettings&& InSettings)
{
	RunCommandOnProcessingThread([this, Settings = MoveTemp(InSettings)]() mutable
	{
		FAtomModulationBusMixHandle BusMixHandle = FAtomModulationBusMixHandle::Get(Settings.GetId(), RefProxies.BusMixes);
		if (BusMixHandle.IsValid())
		{
			BusMixHandle.FindProxy().SetEnabled(MoveTemp(Settings));
		}
		else
		{
			BusMixHandle = FAtomModulationBusMixHandle::Create(MoveTemp(Settings), RefProxies.BusMixes, *this);
		}

		ManuallyActivatedBusMixes.Add(MoveTemp(BusMixHandle));
	});
}

void FAtomModulationSystem::ActivateBusMix(const UAtomModulationControlBusMix& InBusMix)
{
	ActivateBusMix(FAtomModulatorBusMixSettings(InBusMix));
}

void FAtomModulationSystem::ActivateGenerator(const UAtomModulationGenerator& InGenerator)
{
	RunCommandOnProcessingThread([this, Settings = FAtomModulationGeneratorSettings(InGenerator)]() mutable
	{
		FAtomModulationGeneratorHandle GeneratorHandle = FAtomModulationGeneratorHandle::Get(Settings.GetId(), RefProxies.Generators);
		if (GeneratorHandle.IsValid())
		{
			ManuallyActivatedGenerators.Add(MoveTemp(GeneratorHandle));
		}
		else
		{
			GeneratorHandle = FAtomModulationGeneratorHandle::Create(MoveTemp(Settings), RefProxies.Generators, *this);
			GeneratorHandle.FindProxy().Init(AtomRuntimeId);
		}
	});
}

/*bool FAtomModulationSystem::CalculateModulationValue(FAtomModulationPatchProxy& OutProxy, float& OutValue) const
{
	check(IsInProcessingThread());
	if (OutProxy.IsBypassed())
	{
		return false;
	}

	const float InitValue = OutValue;
	OutProxy.Update();
	OutValue = OutProxy.GetValue();
	return !FMath::IsNearlyEqual(InitValue, OutValue);
}*/

void FAtomModulationSystem::DeactivateBus(const UAtomModulationControlBus& InBus)
{
	ClearGlobalBusMixValue(InBus);

	RunCommandOnProcessingThread([this, BusId = static_cast<FAtomModulationBusId>(InBus.GetUniqueID())]()
	{
		FAtomModulationBusHandle BusHandle = FAtomModulationBusHandle::Get(BusId, RefProxies.Buses);
		if (BusHandle.IsValid())
		{
			ManuallyActivatedBuses.Remove(BusHandle);
		}
	});
}

void FAtomModulationSystem::DeactivateBusMix(const UAtomModulationControlBusMix& InBusMix)
{
	RunCommandOnProcessingThread([this, BusMixId = static_cast<FAtomModulationBusMixId>(InBusMix.GetUniqueID())]()
	{
		FAtomModulationBusMixHandle MixHandle = FAtomModulationBusMixHandle::Get(BusMixId, RefProxies.BusMixes);
		if (MixHandle.IsValid())
		{
			FAtomModulatorBusMixProxy& MixProxy = MixHandle.FindProxy();
			MixProxy.SetStopping();
		}
	});
}

void FAtomModulationSystem::DeactivateAllBusMixes()
{
	ClearAllGlobalBusMixValues();

	RunCommandOnProcessingThread([this]()
	{
		for (TPair<FAtomModulationBusMixId, FAtomModulatorBusMixProxy>& Pair : RefProxies.BusMixes)
		{
			Pair.Value.SetStopping();
		}
	});
}

void FAtomModulationSystem::DeactivateGenerator(const UAtomModulationGenerator& InGenerator)
{
	RunCommandOnProcessingThread([this, GeneratorId = static_cast<FAtomModulationGeneratorId>(InGenerator.GetUniqueID())]()
	{
		FAtomModulationGeneratorHandle GeneratorHandle = FAtomModulationGeneratorHandle::Get(GeneratorId, RefProxies.Generators);
		if (GeneratorHandle.IsValid())
		{
			ManuallyActivatedGenerators.Remove(GeneratorHandle);
		}
	});
}

#if !UE_BUILD_SHIPPING
void FAtomModulationSystem::SetDebugBusFilter(const FString* InFilter)
{
	Debugger->SetDebugBusFilter(InFilter);
}

void FAtomModulationSystem::SetDebugGeneratorsEnabled(bool bInIsEnabled)
{
	Debugger->SetDebugGeneratorsEnabled(bInIsEnabled);
}

void FAtomModulationSystem::SetDebugGeneratorFilter(const FString* InFilter)
{
	Debugger->SetDebugGeneratorFilter(InFilter);
}

void FAtomModulationSystem::SetDebugGeneratorTypeFilter(const FString* InFilter, bool bInEnabled)
{
	Debugger->SetDebugGeneratorTypeFilter(InFilter, bInEnabled);
}

void FAtomModulationSystem::SetDebugMatrixEnabled(bool bInIsEnabled)
{
	Debugger->SetDebugMatrixEnabled(bInIsEnabled);
}

void FAtomModulationSystem::SetDebugMixFilter(const FString* InNameFilter)
{
	Debugger->SetDebugMixFilter(InNameFilter);
}
#endif // !UE_BUILD_SHIPPING

void FAtomModulationSystem::SetGlobalBusMixValue(UAtomModulationControlBus& InBus, float InValue, float InFadeTime)
{
	if (TObjectPtr<UAtomModulationControlBusMix> GlobalMix = ActiveGlobalBusValueMixes.FindRef(InBus.GetUniqueID()))
	{
		if (ensure(!GlobalMix->MixStages.IsEmpty()))
		{
			GlobalMix->MixStages[0].Value.TargetValue = InValue;
			UpdateMix(*GlobalMix, InFadeTime);

			UE_LOG(LogAtomModulation, VeryVerbose, TEXT("GlobalBusMix for ControlBus '%s' updated, target set to '%0.4f'."), *InBus.GetName(), InValue);
		}
	}
	else
	{
		const FString MixName = InBus.GetName() + TEXT("_GlobalMix");
		TObjectPtr<UAtomModulationControlBusMix> NewGlobalMix = NewObject<UAtomModulationControlBusMix>(GetTransientPackage(), FName(*MixName));

		{
			FAtomModulationMixValue MixValue;
			MixValue.TargetValue = InValue;

			if (InFadeTime >= 0.0f)
			{
				MixValue.AttackTime = InFadeTime;
			}

			FAtomModulationControlBusMixStage MixStage;
			MixStage.Bus = &InBus;
			MixStage.Value = MixValue;

			NewGlobalMix->MixStages.Emplace(MoveTemp(MixStage));
		}

		ActiveGlobalBusValueMixes.Add(InBus.GetUniqueID(), NewGlobalMix);
		UE_LOG(LogAtomModulation, VeryVerbose, TEXT("GlobalBusMix for ControlBus '%s' activated, target set to '%0.4f'."), *InBus.GetName(), InValue);
		ActivateBusMix(*NewGlobalMix);
		NewGlobalMix->AddToRoot();
	}

}

void FAtomModulationSystem::ClearGlobalBusMixValue(const UAtomModulationControlBus& InBus, float InFadeTime)
{
	const uint32 BusID = InBus.GetUniqueID();
	if (TObjectPtr<UAtomModulationControlBusMix> GlobalMix = ActiveGlobalBusValueMixes.FindRef(BusID))
	{
		if (ensure(!GlobalMix->MixStages.IsEmpty()))
		{
			GlobalMix->MixStages[0].Value.ReleaseTime = InFadeTime;
			DeactivateBusMix(*GlobalMix);
			ActiveGlobalBusValueMixes.Remove(BusID);
			GlobalMix->RemoveFromRoot();
			UE_LOG(LogAtomModulation, VeryVerbose, TEXT("GlobalBusMix for ControlBus '%s' cleared."), *InBus.GetName());
		}
	}
	else
	{
		UE_LOG(LogAtomModulation, VeryVerbose, TEXT("GlobalBusMix for ControlBus '%s' not active, ignoring clear request."), *InBus.GetName());
	}
}

void FAtomModulationSystem::ClearAllGlobalBusMixValues(float InFadeTime)
{
	TArray<TObjectPtr<UAtomModulationControlBusMix>> GlobalBusMixes;
	ActiveGlobalBusValueMixes.GenerateValueArray(GlobalBusMixes);
	for (const TObjectPtr<UAtomModulationControlBusMix>& BusMix : GlobalBusMixes)
	{
		if (ensure(!BusMix->MixStages.IsEmpty()))
		{
			ClearGlobalBusMixValue(*BusMix->MixStages[0].Bus, InFadeTime);
		}
	}

	ActiveGlobalBusValueMixes.Reset();
}

bool FAtomModulationSystem::GetModulatorValue(const FAtomModulatorHandle& InModulatorHandle, float& OutValue) const
{
	const EAtomModulatorType ModulatorType = static_cast<EAtomModulatorType>(InModulatorHandle.GetTypeId());

	switch (ModulatorType)
	{
		case EAtomModulatorType::Patch:
		{
			// Direct access preferred vs through handles here as its impossible for proxies to be destroyed
			// in look-up and speed is key as this is possibly being queried often in the audio render pass.
		//	if (const FAtomModulationPatchRefProxy* PatchProxy = RefProxies.Patches.Find(static_cast<FAtomModulationPatchId>(InModulatorHandle.GetModulatorId())))
		//	{
		//		if (!PatchProxy->IsBypassed())
		//		{
		//			OutValue = PatchProxy->GetValue();
		//			return true;
		//		}
		//	}
		}
		break;

		case EAtomModulatorType::Bus:
		{
			if (const FAtomModulationControlBusProxy* BusProxy = RefProxies.Buses.Find(static_cast<FAtomModulationBusId>(InModulatorHandle.GetModulatorId())))
			{
				if (!BusProxy->IsBypassed())
				{
					OutValue = BusProxy->GetValue();
					return true;
				}
			}
		}
		break;

		case EAtomModulatorType::Generator:
		{
			if (const FAtomModulatorGeneratorProxy* GeneratorProxy = RefProxies.Generators.Find(static_cast<FAtomModulationGeneratorId>(InModulatorHandle.GetModulatorId())))
			{
				if (!GeneratorProxy->IsBypassed())
				{
					OutValue = GeneratorProxy->GetValue();
					return true;
				}
			}
		}
		break;

		default:
		{
			static_assert(static_cast<uint32>(EAtomModulatorType::Count) == 3, "Possible missing modulator type coverage in switch statement");
		}
		break;
	}

	return false;
}

bool FAtomModulationSystem::GetModulatorValueThreadSafe(const FAtomModulatorHandle& InModulatorHandle, float& OutValue) const
{
	FScopeLock Lock(&ThreadSafeModValueCritSection);

	if (const float* Value = ThreadSafeModValueMap.Find(InModulatorHandle.GetModulatorId()))
	{
		OutValue = *Value;
		return true;
	}

	return false;
}

FAtomRuntimeId FAtomModulationSystem::GetAtomRuntimeId() const
{
	return AtomRuntimeId;
}

bool FAtomModulationSystem::IsInProcessingThread() const
{
	return ProcessingThreadId == FPlatformTLS::GetCurrentThreadId();
}

void FAtomModulationSystem::ProcessModulators(const double InElapsed)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FAtomModulationSystem::ProcessModulators);

	// The processing thread can get updated between frames. As modulation
	// processing should be first call in frame, update the threadId accordingly
	ProcessingThreadId = FPlatformTLS::GetCurrentThreadId();

	int32 CommandsProcessed = 0;
	TUniqueFunction<void()> Command;
	while (ProcessingThreadCommandQueue.Dequeue(Command))
	{
		Command();
		++CommandsProcessed;
	}

	TMap<FAtomModulatorId, float> NewModulatorValues;

	// Update Generators (prior to bus mixing to avoid single-frame latency)
	for (TPair<FAtomModulationGeneratorId, FAtomModulatorGeneratorProxy>& Pair : RefProxies.Generators)
	{
		Pair.Value.PumpCommands();
		Pair.Value.Update(InElapsed);
		NewModulatorValues.Add(static_cast<FAtomModulatorId>(Pair.Key), Pair.Value.GetValue());
	}

	// Reset buses & refresh cached Generator
	for (TPair<FAtomModulationBusId, FAtomModulationControlBusProxy>& Pair : RefProxies.Buses)
	{
		Pair.Value.Reset();
		Pair.Value.MixGenerators();
	}

	// Update mix values and apply to prescribed buses.
	// Track bus mixes ready to remove
	TSet<FAtomModulationBusMixId> StoppedMixIds;
	for (TPair<FAtomModulationBusMixId, FAtomModulatorBusMixProxy>& Pair : RefProxies.BusMixes)
	{
		const FAtomModulatorBusMixProxy::EStatus LastStatus = Pair.Value.GetStatus();
		Pair.Value.Update(InElapsed, RefProxies.Buses);
		const FAtomModulatorBusMixProxy::EStatus CurrentStatus = Pair.Value.GetStatus();

		switch (CurrentStatus)
		{
			case FAtomModulatorBusMixProxy::EStatus::Enabled:
			case FAtomModulatorBusMixProxy::EStatus::Stopping:
			break;

			case FAtomModulatorBusMixProxy::EStatus::Stopped:
			{
				if (LastStatus != CurrentStatus)
				{
					UE_LOG(LogAtomModulation, Verbose, TEXT("Atom modulation mix '%s' stopped."), *Pair.Value.GetName());
				}
				StoppedMixIds.Add(Pair.Key);
			}
			break;

			default:
			{
				checkf(false, TEXT("Invalid or unsupported BusMix EStatus state advancement."));
			}
			break;
		}
	}

	// Destroy mixes that have stopped (must be done outside mix update
	// loop above to avoid destroying while iterating, which can occur
	// when update moves bus mix from 'stopping' status to 'stopped')
	for (const FAtomModulationBusMixId& MixId : StoppedMixIds)
	{
		FAtomModulationBusMixHandle MixHandle = FAtomModulationBusMixHandle::Get(MixId, RefProxies.BusMixes);

		// Expected to be valid given the fact that the proxy is available in the prior loop
		check(MixHandle.IsValid());

		// Expected to only have two references (one for transient 'MixHandle' and one in
		// ManuallyActivated set). Nothing else should be keeping mixes active.
		check(MixHandle.FindProxy().GetRefCount() == 2);

		ManuallyActivatedBusMixes.Remove(MoveTemp(MixHandle));
	}

	//for (TPair<FAtomModulationPatchId, FAtomModulationPatchRefProxy>& Pair : RefProxies.Patches)
	//{
	//	FAtomModulationPatchRefProxy& PatchProxy = Pair.Value;
	//	if (!PatchProxy.IsBypassed())
	//	{
	//		PatchProxy.Update();
	//		NewModulatorValues.Add(static_cast<FAtomModulatorId>(Pair.Key), PatchProxy.GetValue());
	//	}
	//}

	for (const TPair<FAtomModulationBusId, FAtomModulationControlBusProxy>& Pair : RefProxies.Buses)
	{
		NewModulatorValues.Add(static_cast<FAtomModulatorId>(Pair.Key), Pair.Value.GetValue());
	}

	{
		FScopeLock Lock(&ThreadSafeModValueCritSection);
		ThreadSafeModValueMap = MoveTemp(NewModulatorValues);
	}

	// Log stats
	SET_DWORD_STAT(STAT_AtomModulationBusCount, RefProxies.Buses.Num());
	SET_DWORD_STAT(STAT_AtomModulationMixCount, RefProxies.BusMixes.Num());
	SET_DWORD_STAT(STAT_AtomModulationGeneratorCount, RefProxies.Generators.Num());
	//SET_DWORD_STAT(STAT_AtomModulationPatchCount, RefProxies.Patches.Num());
	SET_DWORD_STAT(STAT_AtomModulationProcQueueCount, CommandsProcessed);

#if !UE_BUILD_SHIPPING
 	Debugger->UpdateDebugData(InElapsed, RefProxies);
#endif // !UE_BUILD_SHIPPING
}

/*void FAtomModulationSystem::SaveMixToProfile(const UAtomModulationControlBusMix& InBusMix, const int32 InProfileIndex)
{
	check(IsInGameThread());

	RunCommandOnProcessingThread([this, MixToSerialize = TWeakObjectPtr<const UAtomModulationControlBusMix>(&InBusMix), InProfileIndex]()
	{
		if (!MixToSerialize.IsValid())
		{
			return;
		}

		const FAtomModulationControlBusMixId MixId = static_cast<FAtomModulationControlBusMixId>(MixToSerialize->GetUniqueID());
		const FString   MixName = MixToSerialize->GetName();

		FAtomModulationBusMixHandle MixHandle = FAtomModulationBusMixHandle::Get(MixId, RefProxies.BusMixes);
		const bool bIsActive = MixHandle.IsValid();
		if (!MixHandle.IsValid())
		{
			UE_LOG(LogAtomModulation, Display, TEXT("Mix '%s' is inactive, saving default object to profile '%i'."), *MixName, InProfileIndex);
			AsyncTask(ENamedThreads::GameThread, [this, MixToSerialize, InProfileIndex]()
			{
				AudioModulation::FProfileSerializer::Serialize(*MixToSerialize.Get(), InProfileIndex);
			});
			return;
		}

		UE_LOG(LogAtomModulation, Display, TEXT("Mix '%s' is active, saving current mix proxy state to profile '%i'."), *MixName, InProfileIndex);
		AudioModulation::FAtomModulatorBusMixProxy& MixProxy = MixHandle.FindProxy();
		TMap<FAtomModulationControlBusId, FAtomModulationMixValue> PassedStageInfo;
		for (TPair<FAtomModulationControlBusId, FAtomModulatorBusMixStageProxy>& Pair : MixProxy.Stages)
		{
			FAtomModulatorBusMixStageProxy& Stage = Pair.Value;
			PassedStageInfo.Add(Pair.Key, Stage.Value);
		}

		AsyncTask(ENamedThreads::GameThread, [this, PassedStageInfo, MixToSerialize, InProfileIndex]()
		{
			if (!MixToSerialize.IsValid())
			{
				return;
			}
						
			TMap<FAtomModulationControlBusId, FAtomModulationMixValue> StageInfo = PassedStageInfo;
			UAtomModulationControlBusMix* TempMix = NewObject<UAtomModulationControlBusMix>(GetTransientPackage(), *FGuid::NewGuid().ToString(EGuidFormats::Short));

			// Buses on proxy may differ than those on uobject definition, so iterate and find by cached ids
			// and add to temp mix to be serialized.
			for (TObjectIterator<UAtomModulationControlBus> Itr; Itr; ++Itr)
			{
				if (UAtomModulationControlBus* Bus = *Itr)
				{
					FAtomModulationControlBusId ItrBusId = static_cast<FAtomModulationControlBusId>(Bus->GetUniqueID());
					if (FAtomModulationMixValue* Value = StageInfo.Find(ItrBusId))
					{
						FAtomModulationControlBusMixStage BusMixStage;
						BusMixStage.Bus = Bus;
						BusMixStage.Value = *Value;
						TempMix->MixStages.Add(MoveTemp(BusMixStage));
					}
				}
			}

			const FString MixPath = MixToSerialize->GetPathName();
			//AudioModulation::FProfileSerializer::Serialize(*TempMix, InProfileIndex, &MixPath);
		});
	});
}*/

/*TArray<FAtomModulationControlBusMixStage> FAtomModulationSystem::LoadMixFromProfile(const int32 InProfileIndex, UAtomModulationControlBusMix& OutBusMix)
{
	const FString TempName = FGuid::NewGuid().ToString(EGuidFormats::Short);
	if (UAtomModulationControlBusMix* TempMix = NewObject<UAtomModulationControlBusMix>(GetTransientPackage(), *TempName))
	{
		const FString MixPath = OutBusMix.GetPathName();
		AudioModulation::FProfileSerializer::Deserialize(InProfileIndex, *TempMix, &MixPath);
		UpdateMix(TempMix->MixStages, OutBusMix);
		return TempMix->MixStages;
	}

	return TArray<FAtomModulationControlBusMixStage>();
}*/

void FAtomModulationSystem::RunCommandOnProcessingThread(TUniqueFunction<void()> Cmd)
{
	if (IsInProcessingThread())
	{
		Cmd();
	}
	else
	{
		ProcessingThreadCommandQueue.Enqueue(MoveTemp(Cmd));
	}
}

FAtomModulatorTypeId FAtomModulationSystem::RegisterModulator(FAtomModulatorHandleId InHandleId, const FAtomModulationControlBusSettings& InSettings)
{
	FAtomModulationControlBusSettings CachedSettings = InSettings;
	RegisterModulator(InHandleId, MoveTemp(CachedSettings), RefProxies.Buses, RefModulators.BusMap);
	return static_cast<FAtomModulatorTypeId>(EAtomModulatorType::Bus);
}

FAtomModulatorTypeId FAtomModulationSystem::RegisterModulator(FAtomModulatorHandleId InHandleId, const FAtomModulationGeneratorSettings& InSettings)
{
	FAtomModulationGeneratorSettings CachedSettings = InSettings;
	RegisterModulator(InHandleId, MoveTemp(CachedSettings), RefProxies.Generators, RefModulators.GeneratorMap, [this](FAtomModulationGeneratorHandle& NewHandle)
	{
		NewHandle.FindProxy().Init(AtomRuntimeId);
	});
	return static_cast<FAtomModulatorTypeId>(EAtomModulatorType::Generator);
}

//FAtomModulatorTypeId FAtomModulationSystem::RegisterModulator(FAtomModulatorHandleId InHandleId, const FAtomModulationPatchSettings& InSettings)
//{
//	FAtomModulationPatchSettings CachedSettings = InSettings;
//	RegisterModulator(InHandleId, MoveTemp(CachedSettings), RefProxies.Patches, RefModulators.PatchMap);
//	return static_cast<FAtomModulatorTypeId>(EAtomModulatorType::Patch);
//}

void FAtomModulationSystem::RegisterModulator(FAtomModulatorHandleId InHandleId, FAtomModulatorId InModulatorId)
{
	RunCommandOnProcessingThread([this, InHandleId, InModulatorId]()
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FAtomModulationSystem::RegisterModulator);
		
		//FAtomModulationPatchHandle PatchHandle = FAtomModulationPatchHandle::Get(static_cast<FAtomModulationPatchId>(InModulatorId), RefProxies.Patches);
		//if (PatchHandle.IsValid())
		//{
		//	if (FAtomModulatorHandleSet* HandleSet = RefModulators.PatchMap.Find(PatchHandle))
		//	{
		//		HandleSet->Add(InHandleId);
		//	}
		//	return;
		//}

		FAtomModulationBusHandle BusHandle = FAtomModulationBusHandle::Get(static_cast<FAtomModulationBusId>(InModulatorId), RefProxies.Buses);
		if (BusHandle.IsValid())
		{
			if (FAtomModulatorHandleSet* HandleSet = RefModulators.BusMap.Find(BusHandle))
			{
				HandleSet->Add(InHandleId);
			}
			return;
		}

		FAtomModulationGeneratorHandle GeneratorHandle = FAtomModulationGeneratorHandle::Get(static_cast<FAtomModulationGeneratorId>(InModulatorId), RefProxies.Generators);
		if (GeneratorHandle.IsValid())
		{
			if (FAtomModulatorHandleSet* HandleSet = RefModulators.GeneratorMap.Find(GeneratorHandle))
			{
				HandleSet->Add(InHandleId);
			}
			return;
		}

		ensureAlwaysMsgf(false, TEXT("Failed to register modulator handle '%i' with pre-existing modulator '%i'"), static_cast<int32>(InHandleId), static_cast<int32>(InModulatorId));
	});
}

void FAtomModulationSystem::SoloBusMix(const UAtomModulationControlBusMix& InBusMix)
{
	RunCommandOnProcessingThread([this, BusMixSettings = FAtomModulatorBusMixSettings(InBusMix)]() mutable
	{
		bool bMixActive = false;
		for (TPair<FAtomModulationBusMixId, FAtomModulatorBusMixProxy>& Pair : RefProxies.BusMixes)
		{
			if (Pair.Key == BusMixSettings.GetId())
			{
				bMixActive = true;
			}
			else
			{
				Pair.Value.SetStopping();
			}
		}

		if (!bMixActive)
		{
			ActivateBusMix(MoveTemp(BusMixSettings));
		}
	});
}

void FAtomModulationSystem::UnregisterModulator(const FAtomModulatorHandle& InHandle)
{
	RunCommandOnProcessingThread([this, ModId = InHandle.GetModulatorId(), HandleId = InHandle.GetHandleId()]()
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FAtomModulationSystem::UnregisterModulator);

		//FAtomModulationPatchHandle PatchHandle = FAtomModulationPatchHandle::Get(static_cast<FAtomModulationPatchId>(ModId), RefProxies.Patches);
		//if (UnregisterModulator<FAtomModulationPatchHandle>(PatchHandle, RefModulators.PatchMap, HandleId))
		//{
		//	return;
		//}

		FAtomModulationBusHandle BusHandle = FAtomModulationBusHandle::Get(static_cast<FAtomModulationBusId>(ModId), RefProxies.Buses);
		if (UnregisterModulator<FAtomModulationBusHandle>(BusHandle, RefModulators.BusMap, HandleId))
		{
			return;
		}

		FAtomModulationGeneratorHandle GeneratorHandle = FAtomModulationGeneratorHandle::Get(static_cast<FAtomModulationGeneratorId>(ModId), RefProxies.Generators);
		if (UnregisterModulator<FAtomModulationGeneratorHandle>(GeneratorHandle, RefModulators.GeneratorMap, HandleId))
		{
			return;
		}
	});
}

void FAtomModulationSystem::UpdateMix(const TArray<FAtomModulationControlBusMixStage>& InStages, UAtomModulationControlBusMix& InOutMix, bool bInUpdateObject, float InFadeTime)
{
	if (bInUpdateObject)
	{
		TMap<uint32, const FAtomModulationControlBusMixStage*> UpdatedStageBuses;
		for (const FAtomModulationControlBusMixStage& Stage : InStages)
		{
			if (Stage.Bus)
			{
				UpdatedStageBuses.Add(Stage.Bus->GetUniqueID(), &Stage);
			}
		}

		bool bMarkDirty = false;
		for (FAtomModulationControlBusMixStage& Stage : InOutMix.MixStages)
		{
			if (!Stage.Bus)
			{
				continue;
			}

			if (const FAtomModulationControlBusMixStage* BusStage = UpdatedStageBuses.FindRef(Stage.Bus->GetUniqueID()))
			{
				Stage = *BusStage;
				bMarkDirty = true;
			}
		}
		InOutMix.MarkPackageDirty();
	}

	const FAtomModulationBusMixId MixId = static_cast<FAtomModulationBusMixId>(InOutMix.GetUniqueID());

	TArray<FAtomModulatorBusMixStageSettings> StageSettings;
	for (const FAtomModulationControlBusMixStage& Stage : InStages)
	{
		if (Stage.Bus)
		{
			StageSettings.Emplace(Stage);
		}
	}
	
	RunCommandOnProcessingThread([this, MixId, StageSettings, InFadeTime]()
	{
		if (FAtomModulatorBusMixProxy* BusMixes = RefProxies.BusMixes.Find(MixId))
		{
			BusMixes->SetMix(StageSettings, InFadeTime);
		}
	});
}

void FAtomModulationSystem::UpdateMixByFilter(
	const FString& InAddressFilter,
	const TSubclassOf<UAtomModulationParameter>& InParamClassFilter,
	UAtomModulationParameter* InParamFilter,
	float InValue,
	float InFadeTime,
	UAtomModulationControlBusMix& InOutMix,
	bool bInUpdateObject)
{
	const uint32 ParamClassId = InParamClassFilter ? InParamClassFilter->GetUniqueID() : INDEX_NONE;
	const uint32 ParamId = InParamFilter ? InParamFilter->GetUniqueID() : INDEX_NONE;

	if (bInUpdateObject)
	{
		bool bMarkDirty = false;
		for (FAtomModulationControlBusMixStage& Stage : InOutMix.MixStages)
		{
			if (!Stage.Bus)
			{
				continue;
			}

			if (UAtomModulationParameter* Parameter = Stage.Bus->Parameter)
			{
				if (ParamId != INDEX_NONE && ParamId != Parameter->GetUniqueID())
				{
					continue;
				}

				if (UClass* Class = Parameter->GetClass())
				{
					if (ParamClassId != INDEX_NONE && ParamClassId != Class->GetUniqueID())
					{
						continue;
					}
				}
			}

			if (!FAudioAddressPattern::PartsMatch(InAddressFilter, Stage.Bus->Address))
			{
				continue;
			}

			Stage.Value.TargetValue = InValue;
			Stage.Value.SetActiveFade(FAtomModulationMixValue::EActiveFade::Override, InFadeTime);
			bMarkDirty = true;
		}

		if (bMarkDirty)
		{
			InOutMix.MarkPackageDirty();
		}
	}

	const FString	AddressFilter = InAddressFilter;
	const FAtomModulationBusMixId MixId = static_cast<FAtomModulationBusMixId>(InOutMix.GetUniqueID());
	RunCommandOnProcessingThread([this, ParamClassId, ParamId, MixId, AddressFilter, InValue, InFadeTime]()
	{
		if (FAtomModulatorBusMixProxy* MixProxy = RefProxies.BusMixes.Find(MixId))
		{
			MixProxy->SetMixByFilter(AddressFilter, ParamClassId, ParamId, InValue, InFadeTime);
		}
	});
}

void FAtomModulationSystem::UpdateMix(const UAtomModulationControlBusMix& InMix, float InFadeTime)
{
	RunCommandOnProcessingThread([this, MixSettings = FAtomModulatorBusMixSettings(InMix), InFadeTime]() mutable
	{
		FAtomModulationBusMixHandle BusMixHandle = FAtomModulationBusMixHandle::Get(MixSettings.GetId(), RefProxies.BusMixes);
		if (BusMixHandle.IsValid())
		{
			FAtomModulatorBusMixProxy& MixProxy = BusMixHandle.FindProxy();
			if (MixProxy.GetStatus() == FAtomModulatorBusMixProxy::EStatus::Enabled)
			{
				MixProxy = MoveTemp(MixSettings);
				for (TPair<FAtomModulationBusId, FAtomModulatorBusMixStageProxy>& Stage : MixProxy.Stages)
				{
					Stage.Value.Value.SetActiveFade(FAtomModulationMixValue::EActiveFade::Override, InFadeTime);
				}
			}
		}
#if !UE_BUILD_SHIPPING
		else
		{
			UE_LOG(LogAtomModulation, Verbose, TEXT("Update to '%s' Ignored: Control Bus Mix is inactive."), *MixSettings.GetName());
		}
#endif // !UE_BUILD_SHIPPING
	});
}

void FAtomModulationSystem::UpdateModulator(const UAtomModulatorBase& InModulator)
{
	if (const UAtomModulationGenerator* InGenerator = Cast<UAtomModulationGenerator>(&InModulator))
	{
		RunCommandOnProcessingThread([this, GeneratorSettings = FAtomModulationGeneratorSettings(*InGenerator)]() mutable
		{
			FAtomModulationGeneratorHandle GeneratorHandle = FAtomModulationGeneratorHandle::Get(GeneratorSettings.GetId(), RefProxies.Generators);
			if (GeneratorHandle.IsValid())
			{
				GeneratorHandle.FindProxy() = MoveTemp(GeneratorSettings);
			}
#if !UE_BUILD_SHIPPING
			else
			{
				UE_LOG(LogAtomModulation, Verbose, TEXT("Update to '%s' Ignored: Generator is inactive."), *GeneratorSettings.GetName());
			}
#endif // !UE_BUILD_SHIPPING
		});
	}

	if (const UAtomModulationControlBus* InBus = Cast<UAtomModulationControlBus>(&InModulator))
	{
		RunCommandOnProcessingThread([this, BusSettings = FAtomModulationControlBusSettings(*InBus)]() mutable
		{
			FAtomModulationBusHandle BusHandle = FAtomModulationBusHandle::Get(BusSettings.GetId(), RefProxies.Buses);
			if (BusHandle.IsValid())
			{
				FAtomModulationControlBusProxy& BusProxy = BusHandle.FindProxy();
				BusProxy = MoveTemp(BusSettings);
			}
#if !UE_BUILD_SHIPPING
			else
			{
				UE_LOG(LogAtomModulation, Verbose, TEXT("Update to '%s' Ignored: Control Bus is inactive."), *BusSettings.GetName());
			}
#endif // !UE_BUILD_SHIPPING
		});
	}

//	if (const USoundModulationPatch* InPatch = Cast<USoundModulationPatch>(&InModulator))
//	{
//		RunCommandOnProcessingThread([this, PatchSettings = FAtomModulationPatchSettings(*InPatch)]() mutable
//		{
//			FAtomModulationPatchHandle PatchHandle = FAtomModulationPatchHandle::Get(PatchSettings.GetId(), RefProxies.Patches);
//			if (PatchHandle.IsValid())
//			{
//				FAtomModulationPatchRefProxy& PatchProxy = PatchHandle.FindProxy();
//				PatchProxy = MoveTemp(PatchSettings);
//			}
///*#if !UE_BUILD_SHIPPING
//			else
//			{
//				UE_LOG(LogAtomModulation, Verbose, TEXT("Update to '%s' Ignored: Patch is inactive."), *PatchSettings.GetName());
//			}
//#endif // !UE_BUILD_SHIPPING*/
//		});
//	}
}
