
#pragma once

#include "CoreMinimal.h"

#include "AtomModulationLogs.h"

// Forward Declarations
class FAtomModulationSystem;

/*
 * Handle for all ref-counted proxy types, to be used only on the modulator processing thread (not thread safe).
 */
template<typename IdType, typename ProxyType, typename ProxySettings>
class TAtomProxyHandle
{
	IdType Id;
	TMap<IdType, ProxyType>* ProxyMap;

public:
	FORCEINLINE bool IsValid() const
	{
		return static_cast<uint32>(Id) != INDEX_NONE;
	}

	FORCEINLINE IdType GetId() const
	{
		return Id;
	}

	FORCEINLINE ProxyType& FindProxy()
	{
		check(IsValid());
		check(ProxyMap);

		return ProxyMap->FindChecked(Id);
	}

	FORCEINLINE const ProxyType& FindProxy() const
	{
		check(IsValid());
		check(ProxyMap);

		return ProxyMap->FindChecked(Id);
	}

	friend FORCEINLINE uint32 GetTypeHash(const TAtomProxyHandle<IdType, ProxyType, ProxySettings>& InHandle)
	{
		return static_cast<uint32>(InHandle.Id);
	}

	TAtomProxyHandle()
		: Id(static_cast<IdType>(INDEX_NONE))
		, ProxyMap(nullptr)
	{
	}

	TAtomProxyHandle(IdType InId, TMap<IdType, ProxyType>& InProxyMap)
		: Id(InId)
		, ProxyMap(&InProxyMap)
	{
		if (IsValid())
		{
			ProxyType& Proxy = ProxyMap->FindChecked(Id);
			Proxy.IncRef();
		}
	}

	TAtomProxyHandle(const TAtomProxyHandle& InHandle)
		: Id(InHandle.Id)
		, ProxyMap(InHandle.ProxyMap)
	{
		if (IsValid())
		{
			ProxyType& Proxy = ProxyMap->FindChecked(Id);
			Proxy.IncRef();
		}
	}

	TAtomProxyHandle(TAtomProxyHandle&& InHandle)
		: Id(InHandle.Id)
		, ProxyMap(InHandle.ProxyMap)
	{
		// No Inc/DecRef required due to transfer of ownership
		InHandle.Id = static_cast<IdType>(INDEX_NONE);
		InHandle.ProxyMap = nullptr;
	}

	~TAtomProxyHandle()
	{
		if (!ProxyMap || !IsValid())
		{
			return;
		}

		ProxyType& Proxy = ProxyMap->FindChecked(Id);
		const uint32 RefCount = Proxy.DecRef();
		if (RefCount == 0)
		{
#if UE_BUILD_SHIPPING
			UE_LOG(LogAtomModulation, Verbose, TEXT("Proxy released: Id '%u'."), Id);
#else // UE_BUILD_SHIPPING
			UE_LOG(LogAtomModulation, Verbose, TEXT("Proxy '%s' released: Id '%u'."), *Proxy.GetName(), Id);
#endif // !UE_BUILD_SHIPPING
			ProxyMap->Remove(Id);
		}
	}

	/*
	 * Creates a handle to a proxy modulation object tracked in the provided InProxyMap if it exists, otherwise returns invalid handle.
	 */
	static TAtomProxyHandle<IdType, ProxyType, ProxySettings> Get(const IdType ObjectId, TMap<IdType, ProxyType>& OutProxyMap)
	{
		if (ProxyType* Proxy = OutProxyMap.Find(ObjectId))
		{
			check(Proxy->ModSystem);
			return TAtomProxyHandle<IdType, ProxyType, ProxySettings>(ObjectId, OutProxyMap);
		}

		return TAtomProxyHandle<IdType, ProxyType, ProxySettings>();
	}

	/*
	 * Creates a handle to a proxy modulation object tracked in the provided InProxyMap.  Creates new proxy if it doesn't exist.
	 */
	static TAtomProxyHandle<IdType, ProxyType, ProxySettings> Create(ProxySettings&& InSettings, TMap<IdType, ProxyType>& OutProxyMap, FAtomModulationSystem& OutModSystem)
	{
		const IdType ObjectId = static_cast<IdType>(InSettings.GetId());

		TAtomProxyHandle<IdType, ProxyType, ProxySettings> NewHandle = Get(InSettings.GetId(), OutProxyMap);
		if (!NewHandle.IsValid())
		{
			UE_LOG(LogAtomModulation, Verbose, TEXT("Proxy created: Id '%u' for object '%s'."), ObjectId, *InSettings.GetName());
			OutProxyMap.Add(ObjectId, ProxyType(MoveTemp(InSettings), OutModSystem));
			NewHandle = TAtomProxyHandle<IdType, ProxyType, ProxySettings>(ObjectId, OutProxyMap);
		}

		return NewHandle;
	}

	static TAtomProxyHandle<IdType, ProxyType, ProxySettings> Create(ProxySettings&& InSettings, TMap<IdType, ProxyType>& OutProxyMap, FAtomModulationSystem& OutModSystem, TFunction<void(ProxyType&)> OnCreateProxy)
	{
		const IdType ObjectId = static_cast<IdType>(InSettings.GetId());
		TAtomProxyHandle<IdType, ProxyType, ProxySettings> NewHandle = Get(InSettings.GetId(), OutProxyMap);
		if (!NewHandle.IsValid())
		{
			UE_LOG(LogAtomModulation, Verbose, TEXT("Proxy created: Id '%u' for object '%s'."), NewHandle.Id, *InSettings.GetName());
			OutProxyMap.Add(ObjectId, ProxyType(MoveTemp(InSettings), OutModSystem));
			NewHandle = TAtomProxyHandle<IdType, ProxyType, ProxySettings>(ObjectId, OutProxyMap);
			OnCreateProxy(NewHandle.FindProxy());
		}

		return NewHandle;
	}

	FORCEINLINE bool operator ==(const TAtomProxyHandle<IdType, ProxyType, ProxySettings>& InHandle) const
	{
		return InHandle.Id == Id;
	}

	FORCEINLINE TAtomProxyHandle<IdType, ProxyType, ProxySettings>& operator =(const TAtomProxyHandle<IdType, ProxyType, ProxySettings>& InHandle)
	{
		// 1. If local proxy valid prior to move, cache to DecRef
		ProxyType* ProxyToDecRef = nullptr;
		if (IsValid())
		{
			ProxyToDecRef = &ProxyMap->FindChecked(Id);
		}

		// 2. Move internal state
		Id = InHandle.Id;
		ProxyMap = InHandle.ProxyMap;

		// 3. IncRef new data
		if (IsValid())
		{
			ProxyType& Proxy = ProxyMap->FindChecked(Id);
			Proxy.IncRef();
		}

		// 4. DecRef post IncRef to avoid premature destruction if handling same proxy
		if (ProxyToDecRef)
		{
			ProxyToDecRef->DecRef();
		}

		return *this;
	}

	FORCEINLINE TAtomProxyHandle<IdType, ProxyType, ProxySettings>& operator =(TAtomProxyHandle<IdType, ProxyType, ProxySettings>&& InHandle)
	{
		// 1. If local proxy valid prior to move, cache to DecRef
		ProxyType* ProxyToDecRef = nullptr;
		if (IsValid())
		{
			ProxyToDecRef = &ProxyMap->FindChecked(Id);
		}

		// 2. Move internal state
		Id = InHandle.Id;
		ProxyMap = InHandle.ProxyMap;

		// 3. IncRef new data
		if (IsValid())
		{
			ProxyType& Proxy = ProxyMap->FindChecked(Id);
			Proxy.IncRef();
		}

		// 4. DecRef post IncRef to avoid premature destruction if handling same proxy
		if (ProxyToDecRef)
		{
			ProxyToDecRef->DecRef();
		}

		// 5. DecRef handle to move and clear state
		if (InHandle.IsValid())
		{
			ProxyType& Proxy = ProxyMap->FindChecked(InHandle.Id);
			Proxy.DecRef();
		}
		InHandle.Id = static_cast<IdType>(INDEX_NONE);
		InHandle.ProxyMap = nullptr;

		return *this;
	}
};

// Modulator Proxy Templates
template <typename IdType>
class TAtomModulatorBase
{
private:
	IdType Id;

#if !UE_BUILD_SHIPPING
	FString Name;
#endif // !UE_BUILD_SHIPPING

public:
	TAtomModulatorBase()
		: Id(static_cast<IdType>(INDEX_NONE))
	{
	}

	TAtomModulatorBase(const FString& InName, const uint32 InId)
		: Id(static_cast<IdType>(InId))
#if !UE_BUILD_SHIPPING
		, Name(InName)
#endif // !UE_BUILD_SHIPPING
	{
	}

	virtual ~TAtomModulatorBase() = default;

	IdType GetId() const
	{
		return Id;
	}

	// FOR DEBUG USE ONLY (Not available in shipped builds):
	// Provides name of object that generated proxy.
	const FString& GetName() const
	{
#if UE_BUILD_SHIPPING
		static FString Name;
#endif // UE_BUILD_SHIPPING

		return Name;
	}
};

template<typename IdType, typename ProxyType, typename ProxySettings>
class TAtomModulatorProxyRefType : public TAtomModulatorBase<IdType>
{
protected:
	uint32 RefCount;
	FAtomModulationSystem* ModSystem;

public:
	TAtomModulatorProxyRefType()
		: TAtomModulatorBase<IdType>()
		, RefCount(0)
		, ModSystem(nullptr)
	{
	}

	TAtomModulatorProxyRefType(const FString& InName, const uint32 InId, FAtomModulationSystem& OutModSystem)
		: TAtomModulatorBase<IdType>(InName, InId)
		, RefCount(0)
		, ModSystem(&OutModSystem)
	{
	}

	TAtomModulatorProxyRefType(const TAtomModulatorProxyRefType& InProxyRef)
		: TAtomModulatorBase<IdType>(InProxyRef.GetName(), InProxyRef.GetId())
		, RefCount(InProxyRef.RefCount)
		, ModSystem(InProxyRef.ModSystem)
	{
	}

	TAtomModulatorProxyRefType(TAtomModulatorProxyRefType&& InProxyRef)
		: TAtomModulatorBase<IdType>(InProxyRef.GetName(), InProxyRef.GetId())
		, RefCount(InProxyRef.RefCount)
		, ModSystem(InProxyRef.ModSystem)
	{
		InProxyRef.RefCount = 0;
		InProxyRef.ModSystem = nullptr;
	}

	TAtomModulatorProxyRefType& operator=(const TAtomModulatorProxyRefType& InOther)
	{
		RefCount = InOther.RefCount;
		ModSystem = InOther.ModSystem;
		return *this;
	}

	TAtomModulatorProxyRefType& operator=(TAtomModulatorProxyRefType&& InOther)
	{
		RefCount = InOther.RefCount;
		ModSystem = InOther.ModSystem;

		InOther.RefCount = 0;
		InOther.ModSystem = nullptr;

		return *this;
	}

	virtual ~TAtomModulatorProxyRefType()
	{
		check(RefCount == 0);
	}

	uint32 GetRefCount() const
	{
		return RefCount;
	}

protected:
	FORCEINLINE void IncRef()
	{
		RefCount++;
	}

	FORCEINLINE uint32 DecRef()
	{
		check(RefCount != 0);

		RefCount--;
		return RefCount;
	}

private:
	friend class TAtomProxyHandle<IdType, ProxyType, ProxySettings>;
};
