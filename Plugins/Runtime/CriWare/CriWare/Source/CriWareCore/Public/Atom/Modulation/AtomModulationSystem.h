#pragma once

#include "Atom/AtomRuntimeManager.h"
#include "AtomModulation.h"
#include "Atom/Modulation/AtomModulationControlBusProxy.h"
#include "Atom/Modulation/AtomModulationControlBusMixProxy.h"
#include "Atom/Modulation/AtomModulationGeneratorProxy.h"

// Forward Declarations
class UAtomModulationControlBus;
class UAtomModulationControlBusMix;
class UAtomModulationGenerator;

struct FAtomModulationControlBusSettings;
struct FAtomModulationGeneratorSettings;
struct FAtomModulationPatchSettings;

class FAtomModulationInputProxy;
//class FAtomModulationPatchProxy;
//class FAtomModulationPatchRefProxy;
class FAtomModulatorBusMixStageProxy;
class UViewportStatsSubsystem;

using FAtomModulatorHandleSet = TSet<FAtomModulatorHandleId>;

namespace AtomModulation
{
	struct FReferencedProxies
	{
		FAtomModulationBusMixProxyMap BusMixes;
		FAtomModulationBusProxyMap Buses;
		FAtomModulationGeneratorProxyMap Generators;
		//FAtomModulationPatchProxyMap Patches;
	};

	struct FReferencedModulators
	{
		//TMap<FAtomModulationPatchHandle, FAtomModulatorHandleSet> PatchMap;
		TMap<FAtomModulationBusHandle, FAtomModulatorHandleSet> BusMap;
		TMap<FAtomModulationGeneratorHandle, FAtomModulatorHandleSet> GeneratorMap;
	};

#if !UE_BUILD_SHIPPING
	class FAtomModulationDebugger;
#endif
}

class FAtomModulationSystem : public TSharedFromThis<FAtomModulationSystem>
{
public:
	void Initialize(FAtomRuntimeId RumtimeID);

	void ActivateBus(const UAtomModulationControlBus& InBus);
	void ActivateBusMix(FAtomModulatorBusMixSettings&& InSettings);
	void ActivateBusMix(const UAtomModulationControlBusMix& InBusMix);
	void ActivateGenerator(const UAtomModulationGenerator& InGenerator);

	/**
	 * Deactivates respectively typed (i.e. BusMix, Bus, Generator, etc.) object proxy if no longer referenced.
	 * If still referenced, will wait until references are finished before destroying.
	 */
	void DeactivateBus(const UAtomModulationControlBus& InBus);
	void DeactivateBusMix(const UAtomModulationControlBusMix& InBusMix);
	void DeactivateAllBusMixes();
	void DeactivateGenerator(const UAtomModulationGenerator& InGenerator);

	void ProcessModulators(const double InElapsed);
	void SoloBusMix(const UAtomModulationControlBusMix& InBusMix);

	FAtomRuntimeId GetAtomRuntimeId() const;

	/* Register new handle with given a given modulator that may or may not already be active (i.e. registered).
	 * If already registered, depending on modulator type, may or may not refresh proxy based on provided settings.
	 */
	FAtomModulatorTypeId RegisterModulator(FAtomModulatorHandleId InHandleId, const FAtomModulationControlBusSettings& InSettings);
	FAtomModulatorTypeId RegisterModulator(FAtomModulatorHandleId InHandleId, const FAtomModulationGeneratorSettings& InSettings);
	//FAtomModulatorTypeId RegisterModulator(FAtomModulatorHandleId InHandleId, const FAtomModulationPatchSettings& InSettings);

	/* Register new handle with given Id with a modulator that is already active (i.e. registered). Used primarily for copying modulation handles. */
	void RegisterModulator(FAtomModulatorHandleId InHandleId, FAtomModulatorId InModulatorId);

	/* Attempts to get the modulator value from the AudioRender Thread. */
	bool GetModulatorValue(const FAtomModulatorHandle& ModulatorHandle, float& OutValue) const;

	/* Attempts to get the modulator value from any thread (lock contentious).*/
	bool GetModulatorValueThreadSafe(const FAtomModulatorHandle& ModulatorHandle, float& OutValue) const;

	void UnregisterModulator(const FAtomModulatorHandle& InHandle);

	/* Saves mix to .ini profile for fast iterative development that does not require re-cooking a mix */
	//void SaveMixToProfile(const UAtomModulationControlBusMix& InBusMix, const int32 InProfileIndex);

	/* Loads mix from .ini profile for iterative development that does not require re-cooking a mix. Returns copy
	 * of mix stage values saved in profile. */
	//TArray<FAtomModulationControlBusMixStage> LoadMixFromProfile(const int32 InProfileIndex, UAtomModulationControlBusMix& OutBusMix);

	/*
	 * Updates mix/mix by filter, modifying the mix instance if it is active. If bInUpdateObject is true,
	 * updates UObject definition in addition to proxy.
	 */
	void UpdateMix(const TArray<FAtomModulationControlBusMixStage>& InStages, UAtomModulationControlBusMix& InOutMix, bool bInUpdateObject = false, float InFadeTime = -1.0f);
	void UpdateMixByFilter(const FString& InAddressFilter, const TSubclassOf<UAtomModulationParameter>& InParamClassFilter, UAtomModulationParameter* InParamFilter, float Value, float FadeTime, UAtomModulationControlBusMix& InOutMix, bool bInUpdateObject = false);

	/*
	 * Commits any changes from a mix applied to a UObject definition to mix instance if active.
	 */
	void UpdateMix(const UAtomModulationControlBusMix& InMix, float InFadeTime = -1.0f);

	/* Sets the global bus mix value if over the prescribed time. If FadeTime is non-positive, applies the value immediately. */
	void SetGlobalBusMixValue(UAtomModulationControlBus& Bus, float Value, float FadeTime = -1.0f);

	/* Clears the global bus mix value over the prescribed FadeTime. If FadeTime is non-positive, returns to the bus's respective parameter default immediately. */
	void ClearGlobalBusMixValue(const UAtomModulationControlBus& InBus, float FadeTime = -1.0f);

	/* Clears all global bus mix values over the prescribed FadeTime. If FadeTime is non-positive, returns to the bus's respective parameter default immediately. */
	void ClearAllGlobalBusMixValues(float FadeTime = -1.0f);

	/*
	 * Commits any changes from a modulator type applied to a UObject definition
	 * to modulator instance if active (i.e. Control Bus, Control Bus Modulator)
	 */
	void UpdateModulator(const UAtomModulatorBase& InModulator);

	void OnAuditionEnd();

private:
	/* Calculates modulation value, storing it in the provided float reference and returns if value changed */
	//bool CalculateModulationValue(FModulationPatchProxy& OutProxy, float& OutValue) const;

	/* Whether or not caller is in processing thread or not */
	bool IsInProcessingThread() const;

	/* Runs the provided command on the audio render thread (at the beginning of the ProcessModulators call) */
	void RunCommandOnProcessingThread(TUniqueFunction<void()> Cmd);

	/* Template for register calls that move the modulator settings objects onto the Audio Processing Thread & create proxies.
	 * If the proxy is already found, adds provided HandleId as reference to given proxy. Does *not* update the proxy with the
	 * given settings.  If update is desired on an existing proxy, UpdateModulator must be explicitly called.
	 * @tparam THandleType Type of modulator handle used to access the proxy on the Processing Thread
	 * @tparam TModSettings Modulation settings to move and use if proxy construction is required on the Audio Processing Thread
	 * @tparam TMapType MapType used to cache the corresponding proxy id & proxy
	 * @tparam TInitFunction (Optional) Function type used to call on the AudioProcessingThread immediately following proxy construction
	 * @param InHandleId HandleId associated with the proxy to be retrieved/generated.
	 * @param InModSettings ModulatorSettings to be used if construction of proxy is required on Audio Processing Thread
	 * @param OutProxyMap Map to find or add proxy to if constructed
	 * @param InInitFunction Function type used to call on the AudioProcessingThread immediately following proxy generation
	 */
	template <typename THandleType, typename TModSettings, typename TMapType, typename TInitFunction = TUniqueFunction<void(THandleType)>>
	void RegisterModulator(FAtomModulatorHandleId InHandleId, TModSettings&& InModSettings, TMapType& OutProxyMap, TMap<THandleType, FAtomModulatorHandleSet>& OutModMap, TInitFunction InInitFunction = TInitFunction())
	{
		check(InHandleId != INDEX_NONE);

		RunCommandOnProcessingThread([
			this,
			InHandleId,
			ModSettings = MoveTemp(InModSettings),
			InitFunction = MoveTemp(InInitFunction),
			PassedProxyMap = &OutProxyMap,
			PassedModMap = &OutModMap
		]() mutable
		{
			check(PassedProxyMap);
			check(PassedModMap);

			THandleType Handle = THandleType::Create(MoveTemp(ModSettings), *PassedProxyMap, *this);
			PassedModMap->FindOrAdd(Handle).Add(InHandleId);
		});
	}

	template <typename THandleType>
	bool UnregisterModulator(THandleType InModHandle, TMap<THandleType, FAtomModulatorHandleSet>& OutHandleMap, const FAtomModulatorHandleId InHandleId)
	{
		bool bHandleRemoved = false;

		if (!InModHandle.IsValid())
		{
			return bHandleRemoved;
		}

		if (FAtomModulatorHandleSet* HandleSet = OutHandleMap.Find(InModHandle))
		{
			bHandleRemoved = HandleSet->Remove(InHandleId) > 0;
			if (HandleSet->IsEmpty())
			{
				OutHandleMap.Remove(InModHandle);
			}
		}

		return bHandleRemoved;
	}

	AtomModulation::FReferencedProxies RefProxies;

	// Critical section & map of copied, computed modulation values for
	// use from the decoder threads & respective MetaSound getter nodes.
	mutable FCriticalSection ThreadSafeModValueCritSection;
	TMap<FAtomModulatorId, float> ThreadSafeModValueMap;

	TSet<FAtomModulationBusHandle> ManuallyActivatedBuses;
	TSet<FAtomModulationBusMixHandle> ManuallyActivatedBusMixes;
	TSet<FAtomModulationGeneratorHandle> ManuallyActivatedGenerators;

	// Global mixes each containing a single stage for any globally manipulated bus stage value.
	TMap<uint32, TObjectPtr<UAtomModulationControlBusMix>> ActiveGlobalBusValueMixes;

	// Command queue to be consumed on processing thread 
	TQueue<TUniqueFunction<void()>, EQueueMode::Mpsc> ProcessingThreadCommandQueue;

	// Thread modulators are processed on
	TAtomic<uint32> ProcessingThreadId{ 0 };

	// Collection of maps with modulator handles to referencing object ids used by externally managing objects
	AtomModulation::FReferencedModulators RefModulators;

	FAtomRuntimeId AtomRuntimeId = INDEX_NONE;

	// Registered Parameters
	using FParameterRegistry = TMap<FName, FAtomModulationParameter>;
	FParameterRegistry ParameterRegistry;

#if !UE_BUILD_SHIPPING
public:
	void SetDebugBusFilter(const FString* InFilter);
	void SetDebugMixFilter(const FString* InFilter);
	void SetDebugMatrixEnabled(bool bInIsEnabled);
	void SetDebugGeneratorsEnabled(bool bInIsEnabled);
	void SetDebugGeneratorFilter(const FString* InFilter);
	void SetDebugGeneratorTypeFilter(const FString* InFilter, bool bInIsEnabled);
	bool OnPostHelp(FCommonViewportClient* ViewportClient, const TCHAR* Stream);
	int OnRenderStat(UViewportStatsSubsystem* ViewportSubSystem, int32 OffsetX, int32 OffsetY);
	int OnRenderStat(FViewport* Viewport, FCanvas* Canvas, int32 X, int32 Y, const UFont& Font);
	bool OnToggleStat(FCommonViewportClient* ViewportClient, const TCHAR* Stream);

private:
	TSharedPtr<AtomModulation::FAtomModulationDebugger> Debugger;
#endif // !UE_BUILD_SHIPPING

	friend FAtomModulationControlBusProxy;
	friend FAtomModulationInputProxy;
	//friend FAtomModulationPatchProxy;
	//friend FAtomModulationPatchRefProxy;
	friend FAtomModulatorBusMixStageProxy;
};
