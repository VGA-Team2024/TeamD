// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Atom/AtomRuntimeManager.h"
#include "Atom/Modulation/AtomModulation.h"
#include "Atom/Modulation/AtomModulationValue.h"
#include "AtomModulationControlBusProxy.h"
#include "AtomModulationProxy.h"

// Forward Declarations
struct FAtomModulationControlBusMixStage;
class UAtomModulationControlBusMix;
class FAtomModulationSystem;

using FAtomModulationBusMixId = uint32;
extern const FAtomModulationBusMixId InvalidBusMixId;

class FAtomModulatorBusMixStageSettings : public TAtomModulatorBase<FAtomModulationBusId>
{
public:
	FAtomModulatorBusMixStageSettings(const FAtomModulationControlBusMixStage& InStage);

	FString Address;
	uint32 ParamClassId = INDEX_NONE;
	uint32 ParamId = INDEX_NONE;
	FAtomModulationMixValue Value;
	FAtomModulationControlBusSettings BusSettings;
};

class FAtomModulatorBusMixSettings : public TAtomModulatorBase<FAtomModulationBusMixId>
{
public:
	FAtomModulatorBusMixSettings(const UAtomModulationControlBusMix& InBusMix);
	FAtomModulatorBusMixSettings(FAtomModulatorBusMixSettings&& InSettings);

	TArray<FAtomModulatorBusMixStageSettings> Stages;
};

class FAtomModulatorBusMixStageProxy : public TAtomModulatorBase<FAtomModulationBusId>
{
public:

	FAtomModulatorBusMixStageProxy(FAtomModulatorBusMixStageSettings&& InSettings, FAtomModulationSystem& OutModSystem);

	FString Address;
	uint32 ParamClassId = INDEX_NONE;
	uint32 ParamId = INDEX_NONE;
	FAtomModulationMixValue Value;
	FAtomModulationBusHandle BusHandle;
};

class FAtomModulatorBusMixProxy : public TAtomModulatorProxyRefType<FAtomModulationBusMixId, FAtomModulatorBusMixProxy, FAtomModulatorBusMixSettings>
{
public:
	enum class EStatus : uint8
	{
		Enabled,
		Stopping,
		Stopped
	};

	FAtomModulatorBusMixProxy(FAtomModulatorBusMixSettings&& InMix, FAtomModulationSystem& InModSystem);

	FAtomModulatorBusMixProxy& operator=(FAtomModulatorBusMixSettings&& InSettings);

	EStatus GetStatus() const;

	// Resets stage map
	void Reset();

	void SetEnabled(FAtomModulatorBusMixSettings&& InSettings);
	void SetMix(const TArray<FAtomModulatorBusMixStageSettings>& InStages, float InFadeTime);
	void SetMixByFilter(const FString& InAddressFilter, uint32 InParamClassId, uint32 InParamId, float InValue, float InFadeTime);
	void SetStopping();

	void Update(const double Elapsed, FAtomModulationBusProxyMap& ProxyMap);

	using FStageMap = TMap<FAtomModulationBusId, FAtomModulatorBusMixStageProxy>;
	FStageMap Stages;

private:
	EStatus Status;
};

using FAtomModulationBusMixProxyMap = TMap<FAtomModulationBusMixId, FAtomModulatorBusMixProxy>;
using FAtomModulationBusMixHandle = TAtomProxyHandle<FAtomModulationBusMixId, FAtomModulatorBusMixProxy, FAtomModulatorBusMixSettings>;
