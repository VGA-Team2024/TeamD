/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCore
 * File     : AtomEngineSubsystem.h
 *
 ****************************************************************************/

#pragma once

#include "Subsystems/Subsystem.h"
#include "Subsystems/SubsystemCollection.h"

#include "Atom/AtomRuntimeManager.h"

#include "AtomEngineSubsystem.generated.h"

// Forward Declarations
class FAtomRuntime;

namespace Atom
{
	class FAtomSourceManager;
} // namespace

/**
 * UAtomSubsystemCollectionRoot
 * Root UObject used to anchor UAtomEngineSubsystems to the FAtomRuntime.
 */
UCLASS()
class CRIWARECORE_API UAtomSubsystemCollectionRoot final
	: public UObject
{
	GENERATED_BODY()

public:

	/** Set the ID of the owning Atom runtime. */
	FORCEINLINE void SetAtomRuntimeID(FAtomRuntimeId RuntimeID) { OwningRuntimeID = RuntimeID; }

	/** Get the ID of the owning Atom runtime. */
	FAtomRuntimeId GetAtomRuntimeID() const { return OwningRuntimeID; }

protected:

	FAtomRuntimeId OwningRuntimeID = INDEX_NONE;
};

/**
 * UAtomEngineSubsystem
 * Base class for auto instanced and initialized systems that share the lifetime of the Atom runtime.
 */
UCLASS(Abstract)
class CRIWARECORE_API UAtomEngineSubsystem
	: public UDynamicSubsystem
{
	GENERATED_BODY()

public:

	UAtomEngineSubsystem();

	/**
	 * Override to get an update call during AtomRuntime::Update
	 *  Note: This call will happen on the Atom thread
	 */
	virtual void Update() {}

	/** Returns the owning Atom runtime handle */
	virtual FAtomRuntimeHandle GetAtomRuntimeHandle() const final;

	/** Return a mutable version of the source manager associated with the owning device handle */
	virtual Atom::FAtomSourceManager* GetMutableSourceManager() final;

	/** Return the source manager associated with the owning device handle */
	virtual const Atom::FAtomSourceManager* GetSourceManager() const final;

	/** Return a mutable version of the mixer device from the owning device handle */
	virtual FAtomRuntime* GetMutableAtomRuntime() final;

	/** Return the mixer device from the owning device handle */
	virtual const FAtomRuntime* GetAtomRuntime() const final;
};

/**
 * FAtomSubsystemCollection - Subsystem collection specifically targeting UAtomEngineSubsystems
 */
class FAtomSubsystemCollection
	: public FSubsystemCollection<UAtomEngineSubsystem>
{
public:

	template<class InterfaceToCastTo>
	void ForEachSubsystem(TFunctionRef<bool(InterfaceToCastTo*)> InFunction) const
	{
		const TArray<USubsystem*>& AllSubsystems = GetSubsystemArrayInternal(UAtomEngineSubsystem::StaticClass());
		for (USubsystem* Subsystem : AllSubsystems)
		{
			if (InterfaceToCastTo* CastedSystem = Cast<InterfaceToCastTo>(Subsystem))
			{
				InFunction(CastedSystem);
			}
		}
	}
};
