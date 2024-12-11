
#include "Atom/AtomAudioBusSubsystem.h"

#include "DSP/MultithreadedPatching.h"
#include "UObject/UObjectIterator.h"

#include "Atom/Atom.h"
#include "Atom/AtomRuntime.h"
#include "Atom/AtomRuntimeManager.h"
#include "Atom/Mixer/AtomMixerSourceManager.h"

std::atomic<uint32> Atom::FAudioBusKey::InstanceIdCounter = 0;

UAtomAudioBusSubsystem::UAtomAudioBusSubsystem()
{
}

bool UAtomAudioBusSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return !IsRunningDedicatedServer();
}

void UAtomAudioBusSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	UE_LOG(LogCriWareAtom, Log, TEXT("Initializing Atom Audio Bus Subsystem for Atom runtime with ID %d"), GetAtomRuntime()->GetAtomRuntimeID());
	InitDefaultAudioBuses();
}

void UAtomAudioBusSubsystem::Deinitialize()
{
	UE_LOG(LogCriWareAtom, Log, TEXT("Deinitializing Atom Audio Bus Subsystem for Atom runtime with ID %d"), GetAtomRuntime()->GetAtomRuntimeID());
	ShutdownDefaultAudioBuses();
}

void UAtomAudioBusSubsystem::StartAudioBus(Atom::FAudioBusKey InAudioBusKey, int32 InNumChannels, bool bInIsAutomatic)
{
	if (IsInGameThread())
	{
		if (ActiveAudioBuses_GameThread.Contains(InAudioBusKey))
		{
			return;
		}

		FActiveBusData BusData;
		BusData.BusKey = InAudioBusKey;
		BusData.NumChannels = InNumChannels;
		BusData.bIsAutomatic = bInIsAutomatic;

		ActiveAudioBuses_GameThread.Add(InAudioBusKey, BusData);

		FAtomThread::RunCommandOnAtomThread([this, InAudioBusKey, InNumChannels, bInIsAutomatic]()
		{
			GetMutableSourceManager()->StartAudioBus(InAudioBusKey, InNumChannels, bInIsAutomatic);
		});
	}
	else
	{
		// If we're not the game thread, this needs to be on the game thread, so queue up a command to execute it on the game thread
		GetMutableAtomRuntime()->GameThreadMPSCCommand([this, InAudioBusKey, InNumChannels, bInIsAutomatic]
		{
			StartAudioBus(InAudioBusKey, InNumChannels, bInIsAutomatic);
		});
	}
}

void UAtomAudioBusSubsystem::StopAudioBus(Atom::FAudioBusKey InAudioBusKey)
{
	if (IsInGameThread())
	{
		if (!ActiveAudioBuses_GameThread.Contains(InAudioBusKey))
		{
			return;
		}

		ActiveAudioBuses_GameThread.Remove(InAudioBusKey);

		FAtomThread::RunCommandOnAtomThread([this, InAudioBusKey]()
		{
			GetMutableSourceManager()->StopAudioBus(InAudioBusKey);
		});
	}
	else
	{
		// If we're not the game thread, this needs to be on the game thread, so queue up a command to execute it on the game thread
		GetMutableAtomRuntime()->GameThreadMPSCCommand([this, InAudioBusKey]
		{
			StopAudioBus(InAudioBusKey);
		});
	}
}

bool UAtomAudioBusSubsystem::IsAudioBusActive(Atom::FAudioBusKey InAudioBusKey) const
{
	if (IsInGameThread())
	{
		return ActiveAudioBuses_GameThread.Contains(InAudioBusKey);
	}

	check(IsInAtomThread());
	return GetSourceManager()->IsAudioBusActive(InAudioBusKey);
}

Audio::FPatchInput UAtomAudioBusSubsystem::AddPatchInputForAudioBus(Atom::FAudioBusKey InAudioBusKey, int32 InFrames, int32 InChannels, float InGain)
{
	using namespace Audio;
	// This function is supporting adding audio bus patches from multiple threads (AT, ART, GT, and tasks) and is currently
	// depending on a number of places where data lives, which accounts for the complexity here.
	// This code needs a clean up to refactor everything into a true MPSC model, along with an MPSC refactor of the source manager
	// and our command queues. Once we do that we can remove the code which branches based on the thread the request is coming from. 

	Atom::FAtomSourceManager* SourceManager = GetMutableSourceManager();
	check(SourceManager);

	if (IsInGameThread())
	{
		if (ActiveAudioBuses_GameThread.Find(InAudioBusKey))
		{
			FPatchInput PatchInput = GetMutableAtomRuntime()->MakePatch(InFrames, InChannels, InGain);
			FAtomThread::RunCommandOnAtomThread([this, PatchInput, InAudioBusKey]()
			{
				GetMutableSourceManager()->AddPatchInputForAudioBus_AudioThread(InAudioBusKey, PatchInput);
			});
			return PatchInput;
		}
		UE_LOG(LogCriWareAtom, Warning, TEXT("Unable to add a patch output for audio bus because audio bus with object id '%u' and instance id '%u' is not active."), InAudioBusKey.ObjectId, InAudioBusKey.InstanceId);
		return FPatchInput();
	}
	else if (IsInAtomThread())
	{
		FPatchInput PatchInput = GetMutableAtomRuntime()->MakePatch(InFrames, InChannels, InGain);
		GetMutableSourceManager()->AddPatchInputForAudioBus_AudioThread(InAudioBusKey, PatchInput);
		return PatchInput;
	}
	/*else if (GetAtomRuntime()->IsAudioRenderingThread())
	{
		check(SourceManager);

		const int32 NumChannels = GetSourceManager()->GetAudioBusNumChannels(InAudioBusKey);
		if (NumChannels > 0)
		{
			FPatchInput PatchInput = GetMutableAtomRuntime()->MakePatch(InFrames, InChannels, InGain);
			SourceManager->AddPatchInputForAudioBus(InAudioBusKey, PatchInput);
			return PatchInput;
		}

		return FPatchInput();
	}*/
	else
	{
		FPatchInput PatchInput = GetMutableAtomRuntime()->MakePatch(InFrames, InChannels, InGain);
		GetMutableAtomRuntime()->GameThreadMPSCCommand([this, PatchInput, InAudioBusKey]() mutable
		{
			if (ActiveAudioBuses_GameThread.Find(InAudioBusKey))
			{
				FAtomThread::RunCommandOnAtomThread([this, InAudioBusKey, PatchInput = MoveTemp(PatchInput)]()
				{
					GetMutableSourceManager()->AddPatchInputForAudioBus_AudioThread(InAudioBusKey, PatchInput);
				});
			}
			else
			{
				UE_LOG(LogCriWareAtom, Warning, TEXT("Unable to add a patch output for audio bus because audio bus with object id '%u' and instance id '%u' is not active."), InAudioBusKey.ObjectId, InAudioBusKey.InstanceId);
			}
		});
		return PatchInput;
	}
}

Audio::FPatchOutputStrongPtr UAtomAudioBusSubsystem::AddPatchOutputForAudioBus(Atom::FAudioBusKey InAudioBusKey, int32 InFrames, int32 InChannels, float InGain)
{
	// This function is supporting adding audio bus patches from multiple threads (AT, ART, GT, and tasks) and is currently
	// depending on a number of places where data lives, which accounts for the complexity here.
	// This code needs a clean up to refactor everything into a true MPSC model, along with an MPSC refactor of the source manager
	// and our command queues. Once we do that we can remove the code which branches based on the thread the request is coming from. 

	Atom::FAtomSourceManager* SourceManager = GetMutableSourceManager();
	check(SourceManager);

	if (IsInGameThread())
	{
		if (ActiveAudioBuses_GameThread.Find(InAudioBusKey))
		{
			Audio::FPatchOutputStrongPtr PatchOutput = GetMutableAtomRuntime()->MakePatch(InFrames, InChannels, InGain);
			FAtomThread::RunCommandOnAtomThread([this, PatchOutput, InAudioBusKey]()
			{
				GetMutableSourceManager()->AddPatchOutputForAudioBus_AudioThread(InAudioBusKey, PatchOutput);
			});
			return PatchOutput;
		}
		UE_LOG(LogCriWareAtom, Warning, TEXT("Unable to add a patch output for audio bus because audio bus with object id '%u' and instance id '%u' is not active."), InAudioBusKey.ObjectId, InAudioBusKey.InstanceId);
		return nullptr;
	}
	else if (IsInAtomThread())
	{
		Audio::FPatchOutputStrongPtr PatchOutput = GetMutableAtomRuntime()->MakePatch(InFrames, InChannels, InGain);
		SourceManager->AddPatchOutputForAudioBus_AudioThread(InAudioBusKey, PatchOutput);
		return PatchOutput;
	}
	/*else if (GetAtomRuntime()->IsAudioRenderingThread())
	{
		check(SourceManager);

		const int32 NumChannels = GetMutableSourceManager()->GetAudioBusNumChannels(InAudioBusKey);
		if (NumChannels > 0)
		{
			Audio::FPatchOutputStrongPtr PatchOutput = GetMutableAtomRuntime()->MakePatch(InFrames, InChannels, InGain);
			SourceManager->AddPatchOutputForAudioBus(InAudioBusKey, PatchOutput);
			return PatchOutput;
		}

		return nullptr;
	}*/
	else
	{
		Audio::FPatchOutputStrongPtr PatchOutput = GetMutableAtomRuntime()->MakePatch(InFrames, InChannels, InGain);
		GetMutableAtomRuntime()->GameThreadMPSCCommand([this, PatchOutput, InAudioBusKey]() mutable
		{
			if (ActiveAudioBuses_GameThread.Find(InAudioBusKey))
			{
				FAtomThread::RunCommandOnAtomThread([this, InAudioBusKey, PatchOutput = MoveTemp(PatchOutput)]()
				{
					GetMutableSourceManager()->AddPatchOutputForAudioBus_AudioThread(InAudioBusKey, PatchOutput);
				});
			}
			else
			{
				UE_LOG(LogCriWareAtom, Warning, TEXT("Unable to add a patch output for audio bus because audio bus with object id '%u' and instance id '%u' is not active."), InAudioBusKey.ObjectId, InAudioBusKey.InstanceId);
			}
		});
		return PatchOutput;
	}
}

void UAtomAudioBusSubsystem::InitDefaultAudioBuses()
{
	if (!ensure(IsInGameThread()))
	{
		return;
	}

	if (const UCriWareCoreSettings* AtomSettings = GetDefault<UCriWareCoreSettings>())
	{
		TArray<TStrongObjectPtr<UAtomAudioBus>> StaleBuses = DefaultAudioBuses;
		DefaultAudioBuses.Reset();

		//for (const FDefaultAudioBusSettings& BusSettings : AtomSettings->DefaultAudioBuses)
		//{
		//	if (UObject* BusObject = BusSettings.AudioBus.TryLoad())
		//	{
		//		if (UAtomAudioBus* AudioBus = Cast<UAtomAudioBus>(BusObject))
		//		{
		//			const int32 NumChannels = static_cast<int32>(AudioBus->AudioBusChannels) + 1;
		//			StartAudioBus(Atom::FAudioBusKey(AudioBus->GetUniqueID()), NumChannels, false /* bInIsAutomatic */);
		//
		//			TStrongObjectPtr<UAtomAudioBus>AddedBus(AudioBus);
		//			DefaultAudioBuses.AddUnique(AddedBus);
		//			StaleBuses.Remove(AddedBus);
		//		}
		//	}
		//}

		for (TStrongObjectPtr<UAtomAudioBus>& Bus : StaleBuses)
		{
			if (Bus.IsValid())
			{
				StopAudioBus(Atom::FAudioBusKey(Bus->GetUniqueID()));
			}
		}
	}
	else
	{
		UE_LOG(LogCriWareAtom, Error, TEXT("Failed to initialize Default Atom Audio Buses. Atom Settings not found."));
	}
}

void UAtomAudioBusSubsystem::ShutdownDefaultAudioBuses()
{
	if (!ensure(IsInGameThread()))
	{
		return;
	}

	for (TObjectIterator<UAtomAudioBus> It; It; ++It)
	{
		UAtomAudioBus* AudioBus = *It;
		if (AudioBus)
		{
			StopAudioBus(Atom::FAudioBusKey(AudioBus->GetUniqueID()));
		}
	}

	DefaultAudioBuses.Reset();
}
