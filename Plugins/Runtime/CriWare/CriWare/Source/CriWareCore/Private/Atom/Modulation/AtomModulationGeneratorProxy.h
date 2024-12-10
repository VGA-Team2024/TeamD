// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CriWareDefines.h"
#include "Atom/AtomRuntimeManager.h"
#include "Atom/Modulation/AtomModulation.h"
#include "Atom/Modulation/AtomModulationGenerator.h"
#include "Atom/Modulation/AtomModulationProxy.h"


// Forward Declarations
class FAtomModulationSystem;
class FAtomModulatorGeneratorProxy;

struct FAtomModulationGeneratorSettings;

// Modulator Ids
using FAtomModulationGeneratorId = uint32;
extern const FAtomModulationGeneratorId InvalidGeneratorId;

using FAtomModulationGeneratorProxyMap = TMap<FAtomModulationGeneratorId, FAtomModulatorGeneratorProxy>;
using FAtomModulationGeneratorHandle = TAtomProxyHandle<FAtomModulationGeneratorId, FAtomModulatorGeneratorProxy, FAtomModulationGeneratorSettings>;

struct FAtomModulationGeneratorSettings : public TAtomModulatorBase<FAtomModulationGeneratorId>, public IAtomModulatorSettings
{
	FAtomModulationGeneratorPtr Generator;

	FAtomModulationGeneratorSettings() = default;

	FAtomModulationGeneratorSettings(const UAtomModulationGenerator& InGenerator)
		: TAtomModulatorBase<FAtomModulationGeneratorId>(InGenerator.GetName(), InGenerator.GetUniqueID())
		, Generator(InGenerator.CreateInstance())
	{
	}

	FAtomModulationGeneratorSettings(const FAtomModulationGeneratorSettings& InSettings)
		: TAtomModulatorBase<FAtomModulationGeneratorId>(InSettings.GetName(), InSettings.GetId())
		, Generator(InSettings.Generator.IsValid() ? InSettings.Generator->Clone() : nullptr)
	{
	}

	FAtomModulationGeneratorSettings(FAtomModulationGeneratorSettings&& InSettings)
		: TAtomModulatorBase<FAtomModulationGeneratorId>(InSettings.GetName(), InSettings.GetId())
		, Generator(MoveTemp(InSettings.Generator))
	{
	}

	virtual TUniquePtr<IAtomModulatorSettings> Clone() const override
	{
		return TUniquePtr<IAtomModulatorSettings>(new FAtomModulationGeneratorSettings(*this));
	}


	virtual FAtomModulatorId GetModulatorId() const override
	{
		return static_cast<FAtomModulatorId>(GetId());
	}

	virtual const FAtomModulationParameter& GetOutputParameter() const override;
	virtual FAtomModulatorTypeId Register(FAtomModulatorHandleId HandleId, FAtomModulationSystem& InModulation) const override;
};

class FAtomModulatorGeneratorProxy : public TAtomModulatorProxyRefType<FAtomModulationGeneratorId, FAtomModulatorGeneratorProxy, FAtomModulationGeneratorSettings>
{
	FAtomModulationGeneratorPtr Generator;

public:
	FAtomModulatorGeneratorProxy() = default;
	FAtomModulatorGeneratorProxy(FAtomModulationGeneratorSettings&& InSettings, FAtomModulationSystem& InModSystem);

	FAtomModulatorGeneratorProxy& operator =(FAtomModulationGeneratorSettings&& InSettings);

	float GetValue() const
	{
		return Generator->GetValue();
	}

	void Init(FAtomRuntimeId InRuntimeId)
	{
		Generator->Init(InRuntimeId);
	}

	bool IsBypassed() const
	{
		return Generator->IsBypassed();
	}

	void Update(double InElapsed)
	{
		Generator->Update(InElapsed);
	}

	void PumpCommands()
	{
		Generator->PumpCommands();
	}

#if !UE_BUILD_SHIPPING
	TArray<FString> GetDebugCategories() const
	{
		TArray<FString> DebugCategories;
		DebugCategories.Add(TEXT("Name"));
		DebugCategories.Add(TEXT("Ref Count"));

		TArray<FString> GeneratorCategories;
		Generator->GetDebugCategories(GeneratorCategories);
		DebugCategories.Append(GeneratorCategories);

		return DebugCategories;
	}

	TArray<FString> GetDebugValues() const
	{
		TArray<FString> DebugValues;
		DebugValues.Add(GetName());
		DebugValues.Add(FString::FormatAsNumber(GetRefCount()));

		Generator->GetDebugValues(DebugValues);

		return DebugValues;
	}

	const FString& GetDebugName() const
	{
		return Generator->GetDebugName();
	}
#endif // !UE_BUILD_SHIPPING
};
