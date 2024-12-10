
#include "Atom/Mixer/AtomMixerPlayer.h"

#include "Templates/Function.h"

#include "CriWareDefines.h"
#include "CriWareLLM.h"
#include "CriWareTrace.h" // move to Mixer/AtomMixerTrace.h
#include "Atom/Atom.h"
#include "Atom/AtomRuntime.h"
#include "Atom/AtomActiveSound.h"
#include "Atom/AtomSoundCue.h"
#include "Atom/Mixer/AtomMixer.h"
#include "Atom/Mixer/AtomMixerSourceManager.h"

// dev debug defines for output log
//#define ATOM_SOUND_DEV_DEBUG
//#define ATOM_SOUND_STATUS_DEBUG
//#define ATOM_SOUND_INIT_DEBUG

namespace Atom
{
	/*
	 * FAtomExPlayer implementation
	 *****************************************************************************/

	namespace FAtomExPlayer_NativeCallbacks
	{
		extern "C" void CRIAPI OnPlaybackEvent(void* Obj, CriAtomExPlaybackEvent PlaybackEvent, const CriAtomExPlaybackInfoDetail * Info)
		{
			// ADX should not call this function if player is destroyed
			if (FAtomExPlayer* Self = static_cast<FAtomExPlayer*>(Obj))
			{
				Self->HandleNativeOnPlaybackEvent(PlaybackEvent, Info);
			}
		}

		extern "C" void CRIAPI OnCueBlockIndexChanged(void* Obj, CriAtomExPlaybackId PlaybackID, CriAtomExBlockIndex BlockIndex)
		{
			if (FAtomExPlayer* Self = static_cast<FAtomExPlayer*>(Obj))
			{
				Self->HandleNativeOnCueBlockIndexChanged(PlaybackID, BlockIndex);
			}
		}

		extern "C" void CRIAPI OnFilter(void* Obj, CriAtomExPlaybackId PlaybackID, CriAtomPcmFormat Format, CriSint32 NumChannels, CriSint32 NumSamples, void* Data[])
		{
			// ADX do not call this function if player is destroyed
			if (FAtomExPlayer* Self = static_cast<FAtomExPlayer*>(Obj))
			{
				Self->HandleNativePlayerOnFilter(PlaybackID, Format, NumChannels, NumSamples, Data);
			}
		}
	}

	// this occur from atom asr thread / and can happen from User thread too
	void FAtomExPlayer::HandleNativeOnPlaybackEvent(CriAtomExPlaybackEvent PlaybackEvent, const CriAtomExPlaybackInfoDetail* Info)
	{
		if (!Info)
		{
			return;
		}

		FAtomPlaybackId PlaybackID = Info->id;

#ifdef ATOM_SOUND_STATUS_DEBUG
		UE_LOG(LogCriWareAtomMixerDebug, Warning, TEXT("OnPlaybackEvent Player<%p> ID[%d] Event:%s"), ExPlayer.Get(), PlaybackID,
			PlaybackEvent == CRIATOMEX_PLAYBACK_EVENT_ALLOCATE ? TEXT("Alloc") :
			PlaybackEvent == CRIATOMEX_PLAYBACK_EVENT_FROM_NORMAL_TO_VIRTUAL ? TEXT("Virtualize") :
			PlaybackEvent == CRIATOMEX_PLAYBACK_EVENT_FROM_VIRTUAL_TO_NORMAL ? TEXT("Realize") :
			TEXT("Remove"));
#endif

		QueuedEvents.Enqueue({ PlaybackEvent, PlaybackID });
	}

	// this occur from atom render thread
	void FAtomExPlayer::HandleNativeOnCueBlockIndexChanged(CriAtomExPlaybackId PlaybackID, CriAtomExBlockIndex BlockIndex)
	{
		FScopeLock Lock(&PlaybackListenersLock);

		if (TSharedRef<IPlaybackListener>* ListenerPtr = PlaybackListeners.Find(PlaybackID))
		{
			TSharedRef<IPlaybackListener> Listener = *ListenerPtr;
			Listener->OnCueBlockIndexChanged((int32)BlockIndex);
		}
	}

	// this occur from atom render thread
	void FAtomExPlayer::HandleNativePlayerOnFilter(CriAtomExPlaybackId PlaybackID, CriAtomPcmFormat Format, CriSint32 NumChannels, CriSint32 NumFrames, void* Data[])
	{
		FScopeLock Lock(&PlaybackListenersLock);

		if (TSharedRef<IPlaybackListener>* ListenerPtr = PlaybackListeners.Find(PlaybackID))
		{
			TSharedRef<IPlaybackListener> Listener = *ListenerPtr;
			Listener->OnFilter(FAtomRuntime::GetPcmBitDepthFromAtomPcmFormat(Format), NumChannels, NumFrames, Data);
		}
	}

	FAtomExPlayer::FAtomExPlayer()
		: AtomRuntime(nullptr)
		, ActiveSound(nullptr)
		, ExPlayer(nullptr)
		, bIsInitialized(false)
		, bUsePcmFilter(false)
	{
	}

	FAtomExPlayer::FAtomExPlayer(const FAtomExPlayerArgs& InArgs, FCriAtomExPlayerPtr&& ExternalExPlayer)
		: AtomRuntime(InArgs.AtomRuntime)
		, ActiveSound(InArgs.ActiveSound)
		, bIsInitialized(false)
		, bUsePcmFilter(false)
	{
		CriAtomExPlayerConfig Config;
		criAtomExPlayer_SetDefaultConfig(&Config);
		//Config.updates_time = atomconfig; // can get playback time time
		//Config.enable_audio_synced_timer = atomconfig; // can get sample synced time
		//Config.voice_allocation_method = CRIATOMEX_RETRY_VOICE_ALLOCATION; // virtualize if sound not possible

		if (ExternalExPlayer.IsValid())
		{
			ExPlayer = MoveTemp(ExternalExPlayer);
			check(ExPlayer.IsExternal());
			bIsExternal = true;
		}
		else
		{
			ExPlayer = MakeCriHandle(FCriWareApi::criAtomExPlayer_Create(&Config, nullptr, 0));
		}

		if (!ExPlayer)
		{
			// error
			return;
		}

		if (!bIsExternal) // impossible to control lifecycle of the player with external atom player
		{
			// register event callbacks
			FCriWareApi::criAtomExPlayer_SetPlaybackEventCallback(ExPlayer, FAtomExPlayer_NativeCallbacks::OnPlaybackEvent, this);
			FCriWareApi::criAtomExPlayer_SetBlockTransitionCallback(ExPlayer, FAtomExPlayer_NativeCallbacks::OnCueBlockIndexChanged, this);
		}

		bIsInitialized = true;
	}

	FAtomExPlayer::~FAtomExPlayer()
	{
		if (ExPlayer.IsValid())
		{
			if (!bIsExternal)
			{
				// unregister events (To ensure Atom will not call it after player is destroyed!)
				FCriWareApi::criAtomExPlayer_SetPlaybackEventCallback(ExPlayer, nullptr, nullptr);
				FCriWareApi::criAtomExPlayer_SetBlockTransitionCallback(ExPlayer, nullptr, nullptr);
			}

			if (bUsePcmFilter)
			{
				FCriWareApi::criAtomExPlayer_SetFilterCallback(ExPlayer, nullptr, nullptr);
				bUsePcmFilter = false;
			}
		}

		ExPlayer.Reset();
	}

	void FAtomExPlayer::Update()
	{
		FPlaybackEventInfo EventInfo;
		while (QueuedEvents.Dequeue(EventInfo))
		{
			ProcessEvent(EventInfo);
		}
	}

	void FAtomExPlayer::ProcessEvent(FPlaybackEventInfo EventInfo)
	{
        check(IsInAtomThread());
        
		CriAtomExPlaybackEvent PlaybackEvent = EventInfo.Event;
		FAtomPlaybackId PlaybackID = EventInfo.PlaybackID;

		// DEV NOTES:
		//
		// Info.id is ExPlaybackId
		// Info.player is the caller ExPlayer of this FAtomExPlayer.
		//
		// Atom class mapping:
		// -> CriAtomExPlayer (static cue settings, overall playback control)	==> Encapsulated into FAtomExPlayer/FMixerPlayer	this is the main player unit that control sub-playbacks. It is shared between ActiveSound and FAtomExPlayback
		// --> CriAtomExPlaybackId (dsp effects, per playback id control)		==> Encapsulated into FAtomExPlayback/FMixerSource	this are real playback controller from Atom SDK and used by Atom runtime as playback sub-unit
		// ---> CriAtomPlayer (aka voice) (per voice playback control)			==> NOT USED (FAtomPlayer/FMixerSource is used for pure voice playback)
		//
		// From Info we can know with element is allocated or created, removed etc. and inform UE layer when it need to stop or continue ActiveSound. 
		// An active sound control multilple playaback instances that are parameters for each mixer sound source that derives to FAtomExPlayeback object.
		//
		// UE design mapping:
		// The goal is to control, in real time, any sound statuses and parameters then inform the game about any change over time that may hapend (virtualized, realized, stopped, paused, restarted etc.).
		// This may be triggerd by game actions or by the cue itself from sound designer. 
		// 
		// -> AtomComponent			==> the first UObject to play and control one or more sounds.
		// --> ActiveSound			==> instance of a sound/cue/mana sound stream/generated sound. any sound that can be played and controlled by the game. [AC controls multiple AS].
		// ---> PlaybackInstances	==> that are real time parameters for a sound. [AS controls multiple PI].
		// 
		// PlaybackInstance is assigned to one FAtomSoundSource the smallest control/param element in unreal -> an FAtomSource derives to FMixerSource and then to FAtomExPlayback (multi-voices sound, cue...) or FAtomPlayer (single voice).
		// ActiveSound and FAtomSource are assigned to the corresponding FAtomExPlayer that conrols them.
		// 
		// FAtomSoundSources are pooled and managed by AtomMixerSourceManager.
		// ActiveSounds are created from c++ or AtomComponent for Gameplay usage depending situation. 
		// Concurrency system works on ActiveSound layer.

		if (!ActiveSound || !ExPlayer.IsValid())
		{
			// sound was invalidated - nothing to do
			return;
		}

		auto ProcessPlaybackInstance = [this, PlaybackID, PlaybackEvent](bool bCanCreate)
		{
			{ // if exists, redirect to each playback listener
				FScopeLock Lock(&PlaybackListenersLock);
				TSharedRef<IPlaybackListener>* ListenerPtr = PlaybackListeners.Find(PlaybackID);
				if (ListenerPtr)
				{
					TSharedRef<IPlaybackListener> Listener = *ListenerPtr;
					Listener->OnPlaybackEvent((IPlaybackListener::EPlaybackEvent)PlaybackEvent);
					return;
				}
				if (PlaybackListeners.IsEmpty())
				{
					// no yet listeners - first
					return;
				}
			}

			if (bCanCreate)
			{
				// this PlaybackID is unkown so we creates a new playback instance 
				// that will be associated to a free FAtomSource/FAtomExPlayback by AtomRuntime.

				UPTRINT ParentHash = 0;
				const UPTRINT ChildHash = static_cast<UPTRINT>(PlaybackID);

				// Get the PlaybackInstance to use or create new one.
				FAtomPlaybackInstance* ChildPlaybackInstance = ActiveSound->FindPlaybackInstance(ChildHash);
				if (!ChildPlaybackInstance)
				{
					ChildPlaybackInstance = &ActiveSound->AddPlaybackInstance(ChildHash);
				}

				// setup info as soon as possible
				if (ChildPlaybackInstance)
				{
					GetPlaybackInfo(ChildPlaybackInstance, PlaybackID);
				}
			}
		};

		auto VirtualizeActiveSound = [this](FAtomActiveSound& ActiveSoundRef, bool bVirtualize, const TCHAR* ReasonLog)
		{
			if (bVirtualize)
			{
				const bool bDoRangeCheck = false;
				FAtomVirtualLoop VirtualLoop;
				if (FAtomVirtualLoop::Virtualize(ActiveSoundRef, bDoRangeCheck, VirtualLoop))
				{
					if (UAtomSoundBase* Sound = ActiveSoundRef.GetSound())
					{
						UE_LOG(LogCriWareAtomMixer, Display, TEXT("Playing ActiveSound %s Virtualizing: %s"), *Sound->GetName(), ReasonLog);
					}
					ActiveSoundRef.GetAtomRuntime()->AddVirtualLoop(VirtualLoop);
				}
			}
			else
			{
				if (UAtomSoundBase* Sound = ActiveSoundRef.GetSound())
				{
					UE_LOG(LogCriWareAtomMixer, Display, TEXT("Playing ActiveSound %s Realizing: %s"), *Sound->GetName(), ReasonLog);
				}
				ActiveSoundRef.GetAtomRuntime()->RealizeVirtualLoop(ActiveSoundRef);
			}
		};

		// only Atom cues can generate instances
		bool bCanCreate = true;

		if (PlaybackEvent == CRIATOMEX_PLAYBACK_EVENT_ALLOCATE)
		{
			ProcessPlaybackInstance(bCanCreate);
			VirtualizeActiveSound(*ActiveSound, true, TEXT("by allocation"));
		}
		else if (PlaybackEvent == CRIATOMEX_PLAYBACK_EVENT_FROM_NORMAL_TO_VIRTUAL)
		{
			ProcessPlaybackInstance(bCanCreate);
			VirtualizeActiveSound(*ActiveSound, true, TEXT("Sound's voice stollen due to Atom concurrency group maximum met or silent."));
		}
		else if (PlaybackEvent == CRIATOMEX_PLAYBACK_EVENT_FROM_VIRTUAL_TO_NORMAL)
		{
			ProcessPlaybackInstance(bCanCreate);
			VirtualizeActiveSound(*ActiveSound, false, TEXT("Sound's voice is available or sound became audible."));
		}
		else  // CRIATOMEX_PLAYBACK_EVENT_REMOVE
		{
			ProcessPlaybackInstance(false);
		}
	}

	void FAtomExPlayer::RegisterPlaybackListener(TSharedRef<IPlaybackListener> PlaybackListener, FAtomPlaybackId PlaybackID)
	{
		FScopeLock Lock(&PlaybackListenersLock);
		check(!PlaybackListeners.Contains(PlaybackID));
		PlaybackListeners.Emplace(PlaybackID, PlaybackListener);
	}

	void FAtomExPlayer::UnregisterPlaybackListener(FAtomPlaybackId PlaybackID)
	{
		FScopeLock Lock(&PlaybackListenersLock);
		check(PlaybackListeners.Contains(PlaybackID));
		PlaybackListeners.Remove(PlaybackID);
	}

	void FAtomExPlayer::EnablePcmFilter(bool bEnable)
	{
		if (bEnable)
		{
			if (!bUsePcmFilter)
			{
				bUsePcmFilter = true;
				FCriWareApi::criAtomExPlayer_SetFilterCallback(ExPlayer, FAtomExPlayer_NativeCallbacks::OnFilter, this);
			}
		}
		else
		{
			FCriWareApi::criAtomExPlayer_SetFilterCallback(ExPlayer, nullptr, nullptr);
		}
	}

	TSharedPtr<FAtomExPlayer, ESPMode::ThreadSafe> FAtomExPlayer::Create(const FAtomExPlayerArgs& InArgs)
	{
		LLM_SCOPE_CRIWARE(ELLMTagCriWare::AtomMixer);

		// Fail if the Wave has been flagged to contain an error
		//if (InArgs.SoundData /* && InArgs.SoundData->HasError()*/)
		//{
		//	UE_LOG(LogCriWareAtomMixer, VeryVerbose, TEXT("FMixerSourceBuffer::Create failed as '%s' is flagged as containing errors"), *InArgs.SoundData->GetName());
		//	return {};
		//}

		if (InArgs.ActiveSound)
		{
			// find if existing explayer for this activesound
			if (!InArgs.ActiveSound->InstancePlayer.IsValid())
			{
				InArgs.ActiveSound->InstancePlayer = MakeShared<FAtomExPlayer>(InArgs);
			}

			return InArgs.ActiveSound->InstancePlayer;
		}

		return MakeShared<FAtomExPlayer>(InArgs);
	}

	TSharedPtr<FAtomExPlayer, ESPMode::ThreadSafe> FAtomExPlayer::CreateWithExternalPlayer(const FAtomExPlayerArgs& InArgs, FCriAtomExPlayerPtr&& ExternalPlayer)
	{
		LLM_SCOPE_CRIWARE(ELLMTagCriWare::AtomMixer);

		// Fail if the sdk player is invalid
		if (!ExternalPlayer.IsValid())
		{
			UE_LOG(LogCriWareAtomMixer, VeryVerbose, TEXT("FAtomExPlayer::Create failed as '%s' uses an invalid external player."), *InArgs.SoundData->GetName());
			return {};
		}
		
		// Fail if the Wave has been flagged to contain an error
		//if (InArgs.SoundData /* && InArgs.SoundData->HasError()*/)
		//{
		//	UE_LOG(LogCriWareAtomMixer, VeryVerbose, TEXT("FAtomExPlayer::Create failed as '%s' is flagged as containing errors."), *InArgs.SoundData->GetName());
		//	return {};
		//}

		if (InArgs.ActiveSound)
		{
			// find if existing explayer for this activesound
			if (!InArgs.ActiveSound->InstancePlayer.IsValid())
			{
				InArgs.ActiveSound->InstancePlayer = MakeShared<FAtomExPlayer>(InArgs, MoveTemp(ExternalPlayer));
			}

			return InArgs.ActiveSound->InstancePlayer;
		}

		return MakeShared<FAtomExPlayer>(InArgs, MoveTemp(ExternalPlayer));
	}

	void FAtomExPlayer::GetPlaybackInfo(FAtomPlaybackInstance* PlaybackInstance, FAtomPlaybackId PlaybackID)
	{
		if (PlaybackInstance && PlaybackInstance->SoundInfo.SoundID == INDEX_NONE)
		{
			// todo wave info update
			//FCriWareApi::criAtomExPlayback_GetFormatInfo(PlaybackID, &Info);

			CriAtomExSourceInfo SourceInfo;
			FCriWareApi::criAtomExPlayback_GetSource(PlaybackID, &SourceInfo);

			auto& SoundInfo = PlaybackInstance->SoundInfo;

			switch (SourceInfo.type)
			{
			case CRIATOMEX_SOURCE_TYPE_CUE_ID:
			{
				CriAtomExCueInfo Info;
				FCriWareApi::criAtomExAcb_GetCueInfoById(SourceInfo.info.cue_id.acb, SourceInfo.info.cue_id.id, &Info);
				SoundInfo.Type = EAtomSoundPlaybackType::Cue;
				SoundInfo.SoundID = (int32)Info.id;
				SoundInfo.SoundName = UTF8_TO_TCHAR(Info.name);
				break;
			}
			case CRIATOMEX_SOURCE_TYPE_CUE_NAME:
			{
				CriAtomExCueInfo Info;
				FCriWareApi::criAtomExAcb_GetCueInfoByName(SourceInfo.info.cue_name.acb, SourceInfo.info.cue_name.name, &Info);
				SoundInfo.Type = EAtomSoundPlaybackType::Cue;
				SoundInfo.SoundID = (int32)Info.id;
				SoundInfo.SoundName = UTF8_TO_TCHAR(Info.name);
				break;
			}
			case CRIATOMEX_SOURCE_TYPE_CUE_INDEX:
			{
				CriAtomExCueInfo Info;
				FCriWareApi::criAtomExAcb_GetCueInfoByIndex(SourceInfo.info.cue_index.acb, SourceInfo.info.cue_index.index, &Info);
				SoundInfo.Type = EAtomSoundPlaybackType::Cue;
				SoundInfo.SoundID = (int32)Info.id;
				SoundInfo.SoundName = UTF8_TO_TCHAR(Info.name);
				break;
			}
			case CRIATOMEX_SOURCE_TYPE_WAVE_ID:
			{
				// todo use the awb point to retrieve the name of the bank in editor.
				SoundInfo.Type = EAtomSoundPlaybackType::Wave;
				SoundInfo.SoundID = (int32)SourceInfo.info.wave_id.id;
				SoundInfo.SoundName = FName(TEXT("Wave") + FString::FromInt(SourceInfo.info.wave_id.id));
				break;
			}
			case CRIATOMEX_SOURCE_TYPE_RAW_PCM_FLOAT_ID:
				SoundInfo.Type = EAtomSoundPlaybackType::RawPCM;
				SoundInfo.SoundID = 0;
				SoundInfo.SoundName = TEXT("External PCM");
				break;
			case CRIATOMEX_SOURCE_TYPE_INPUT_PORT:
				// todo retrieve the port name
				SoundInfo.Type = EAtomSoundPlaybackType::InputPort;
				SoundInfo.SoundID = 0;
				SoundInfo.SoundName = TEXT("InputPort PCM");
				break;
			default:
				SoundInfo.Type = EAtomSoundPlaybackType::Unknown;
				SoundInfo.SoundID = 0;
				SoundInfo.SoundName = TEXT("Unknown");
				break;
			}
		}
	}

} // namespace
