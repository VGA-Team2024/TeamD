
#pragma once

#include "Templates/TypeHash.h"
#include "UObject/StrongObjectPtr.h"

#include "Atom/AtomEngineSubsystem.h"

#include "AtomAudioBusSubsystem.generated.h"

// Forward Declarations
class UAtomAudioBus;

// Borrow multithreaded Audio patch system from Unreal DSP module
namespace Audio
{
	class FPatchInput;
	struct FPatchOutput;
	typedef TSharedPtr<FPatchOutput, ESPMode::ThreadSafe> FPatchOutputStrongPtr;
} // namespace

namespace Atom
{
	// Forward declarations 
	class FMixerAudioBus;
	class FMixerSourceManager;

	struct CRIWARECORE_API FAudioBusKey
	{
	public:

		uint32 ObjectId = INDEX_NONE; // from a corresponding UObject (UAtomAudioBus) if applicable
		uint32 InstanceId = INDEX_NONE;

		FAudioBusKey()
			: InstanceId(InstanceIdCounter++)
		{
		}

		// For construction with a given UObject unique id 
		FAudioBusKey(uint32 InObjectId)
			: ObjectId(InObjectId)
		{
		}

		const bool IsValid() const
		{
			return ObjectId != INDEX_NONE || InstanceId != INDEX_NONE;
		}

		FORCEINLINE friend uint32 GetTypeHash(const FAudioBusKey& Key)
		{
			return HashCombineFast(Key.ObjectId, Key.InstanceId);
		}
		 		
		FORCEINLINE friend bool operator==(const FAudioBusKey& InLHS, const FAudioBusKey& InRHS) 
		{
			return (InLHS.ObjectId == InRHS.ObjectId) && (InLHS.InstanceId == InRHS.InstanceId);
		}

		FORCEINLINE friend bool operator!=(const FAudioBusKey& InLHS, const FAudioBusKey& InRHS) 
		{
			return !(InLHS == InRHS);
		}
		
	private:

		static std::atomic<uint32> InstanceIdCounter;
	};
} // namespace

/**
*  UAtomAudioBusSubsystem
*/
UCLASS()
class CRIWARECORE_API UAtomAudioBusSubsystem : public UAtomEngineSubsystem
{
	GENERATED_BODY()

public:

	UAtomAudioBusSubsystem();
	virtual ~UAtomAudioBusSubsystem() = default;

	//~ Begin USubsystem interface
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem interface

	// Audio bus API from FMixerDevice
	void StartAudioBus(Atom::FAudioBusKey InAudioBusKey, int32 InNumChannels, bool bInIsAutomatic);
	void StopAudioBus(Atom::FAudioBusKey InAudioBusKey);
	bool IsAudioBusActive(Atom::FAudioBusKey InAudioBusKey) const;
	
	Audio::FPatchInput AddPatchInputForAudioBus(Atom::FAudioBusKey InAudioBusKey, int32 InFrames, int32 InChannels, float InGain = 1.f);
	Audio::FPatchOutputStrongPtr AddPatchOutputForAudioBus(Atom::FAudioBusKey InAudioBusKey, int32 InFrames, int32 InChannels, float InGain = 1.f);

	void InitDefaultAudioBuses();
	void ShutdownDefaultAudioBuses();

private:

	struct FActiveBusData
	{
		Atom::FAudioBusKey BusKey = 0;
		int32 NumChannels = 0;
		bool bIsAutomatic = false;
	};

	TArray<TStrongObjectPtr<UAtomAudioBus>> DefaultAudioBuses; 
	// The active audio bus list accessible on the game thread
	TMap<Atom::FAudioBusKey, FActiveBusData> ActiveAudioBuses_GameThread;
};
