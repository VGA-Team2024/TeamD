/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : CriWareMovieScene
 * File     : MovieSceneAtomTemplate.cpp
 *
 ****************************************************************************/

#include "MovieSceneAtomTemplate.h"

#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "MovieScene.h"
#include "Evaluation/MovieSceneEvaluation.h"
#include "IMovieScenePlayer.h"
#include "GameFramework/WorldSettings.h"
#include "Channels/MovieSceneAudioTriggerChannel.h"

#include "Atom/AtomRuntimeManager.h"
#include "Atom/AtomRuntime.h"
#include "Atom/AtomComponent.h"
#include "Atom/AtomSoundBase.h"
#include "Atom/AtomSoundSimple.h"

#include "MovieSceneAtomSection.h"
#include "MovieSceneAtomTrack.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MovieSceneAtomTemplate)

DECLARE_CYCLE_STAT(TEXT("Atom Track Evaluate"), MovieSceneEval_AtomTrack_Evaluate, STATGROUP_MovieSceneEval);
DECLARE_CYCLE_STAT(TEXT("Atom Track Token Execute"), MovieSceneEval_AtomTrack_TokenExecute, STATGROUP_MovieSceneEval);

static float MaxSequenceAtomDesyncToleranceCVar = 0.5f;
FAutoConsoleVariableRef CVarMaxSequenceAtomDesyncTolerance(
	TEXT("Sequencer.Atom.MaxDesyncTolerance"),
	MaxSequenceAtomDesyncToleranceCVar,
	TEXT("Controls how many seconds an atom track can be out of sync in a Sequence before we attempt a time correction.\n"),
	ECVF_Default);

static bool bIgnoreAtomSyncDuringWorldTimeDilationCVar = true;
FAutoConsoleVariableRef CVarIgnoreAtomSyncDuringWorldTimeDilation(
	TEXT("Sequencer.Atom.IgnoreAtomSyncDuringWorldTimeDilation"),
	bIgnoreAtomSyncDuringWorldTimeDilationCVar,
	TEXT("Ignore correcting audio if there is world time dilation.\n"),
	ECVF_Default);

static int32 UseAtomClockForSequencerDesyncCVar = 0;
FAutoConsoleVariableRef CVaUseAtomClockForSequencerDesync(
	TEXT("Sequencer.Atom.UseAtomClockForAtomDesync"),
	UseAtomClockForSequencerDesyncCVar,
	TEXT("When set to 1, we will use the audio render thread directly to query whether audio has went out of sync with the sequence.\n"),
	ECVF_Default);


/** Stop audio from playing */
struct FStopAtomPreAnimatedToken : IMovieScenePreAnimatedToken
{
	static FMovieSceneAnimTypeID GetAnimTypeID()
	{
		return TMovieSceneAnimTypeID<FStopAtomPreAnimatedToken>();
	}

	virtual void RestoreState(UObject& InObject, const UE::MovieScene::FRestoreStateParams& Params) override
	{
		UAtomComponent* AtomComponent = CastChecked<UAtomComponent>(&InObject);
		if (AtomComponent)
		{
			AtomComponent->Stop();
			AtomComponent->DestroyComponent();
		}
	}

	struct FProducer : IMovieScenePreAnimatedTokenProducer
	{
		virtual IMovieScenePreAnimatedTokenPtr CacheExistingState(UObject& Object) const override
		{
			return FStopAtomPreAnimatedToken();
		}
	};
};

/** Destroy a transient atom component */
struct FDestroyAtomPreAnimatedToken : IMovieScenePreAnimatedToken
{
	static FMovieSceneAnimTypeID GetAnimTypeID()
	{
		return TMovieSceneAnimTypeID<FDestroyAtomPreAnimatedToken>();
	}

	virtual void RestoreState(UObject& InObject, const UE::MovieScene::FRestoreStateParams& Params) override
	{
		UAtomComponent* AtomComponent = CastChecked<UAtomComponent>(&InObject);
		if (AtomComponent)
		{
			AtomComponent->DestroyComponent();
		}
	}

	struct FProducer : IMovieScenePreAnimatedTokenProducer
	{
		virtual IMovieScenePreAnimatedTokenPtr CacheExistingState(UObject& Object) const override
		{
			return FDestroyAtomPreAnimatedToken();
		}
	};
};

struct FCachedAtomTrackData : IPersistentEvaluationData
{
	TMap<FName, FMoveSceneAudioTriggerState> TriggerStateMap;

	TMap<FObjectKey, TMap<FObjectKey, TWeakObjectPtr<UAtomComponent>>> AtomComponentsByActorKey;

	FCachedAtomTrackData()
	{
		// Create the container for root tracks, which do not have an actor to attach to
		AtomComponentsByActorKey.Add(FObjectKey(), TMap<FObjectKey, TWeakObjectPtr<UAtomComponent>>());
	}

	/** Set whenever we ask the atom component to start playing a sound. Used to detect desyncs caused when Sequencer evaluates at more-than-real-time. */
	TMap<TWeakObjectPtr<UAtomComponent>, float> SoundLastPlayedAtTime;

	UAtomComponent* GetAtomComponent(FObjectKey ActorKey, FObjectKey SectionKey)
	{
		if (TMap<FObjectKey, TWeakObjectPtr<UAtomComponent>>* Map = AtomComponentsByActorKey.Find(ActorKey))
		{
			// First, check for an exact match for this section
			TWeakObjectPtr<UAtomComponent> ExistingComponentPtr = Map->FindRef(SectionKey);
			if (ExistingComponentPtr.IsValid())
			{
				return ExistingComponentPtr.Get();
			}

			// If no exact match, check for any AtomComponent that isn't busy
			for (TPair<FObjectKey, TWeakObjectPtr<UAtomComponent >> Pair : *Map)
			{
				UAtomComponent* ExistingComponent = Map->FindRef(Pair.Key).Get();
				if (ExistingComponent && !ExistingComponent->IsPlaying())
				{
					// Replace this entry with the new SectionKey to claim it
					Map->Remove(Pair.Key);
					Map->Add(SectionKey, ExistingComponent);
					return ExistingComponent;
				}
			}
		}

		return nullptr;
	}

	/** Only to be called on the game thread */
	UAtomComponent* AddAtomComponentForRow(int32 RowIndex, FObjectKey SectionKey, UObject& PrincipalObject, IMovieScenePlayer& Player)
	{
		FObjectKey ObjectKey(&PrincipalObject);

		if (!AtomComponentsByActorKey.Contains(ObjectKey))
		{
			AtomComponentsByActorKey.Add(ObjectKey, TMap<FObjectKey, TWeakObjectPtr<UAtomComponent>>());
		}

		UAtomComponent* ExistingComponent = GetAtomComponent(ObjectKey, SectionKey);
		if (!ExistingComponent)
		{
			UAtomSoundSimple* TempPlaybackAtomCue = NewObject<UAtomSoundSimple>();

			AActor* Actor = nullptr;
			USceneComponent* SceneComponent = nullptr;
			FString ObjectName;

			if (PrincipalObject.IsA<AActor>())
			{
				Actor = Cast<AActor>(&PrincipalObject);
				SceneComponent = Actor->GetRootComponent();
				ObjectName =
#if WITH_EDITOR
					Actor->GetActorLabel();
#else
					Actor->GetName();
#endif
			}
			else if (PrincipalObject.IsA<UActorComponent>())
			{
				UActorComponent* ActorComponent = Cast<UActorComponent>(&PrincipalObject);
				Actor = ActorComponent->GetOwner();
				SceneComponent = Cast<USceneComponent>(ActorComponent);
				ObjectName = ActorComponent->GetName();
			}

			if (!Actor || !SceneComponent)
			{
				UE_LOG(LogMovieScene, Warning, TEXT("Failed to find scene component for spatialized atom track (row %d)."), RowIndex);
				return nullptr;
			}

			FAtomRuntime::FCreateComponentParams Params(Actor->GetWorld(), Actor);
			ExistingComponent = FAtomRuntime::CreateComponent(TempPlaybackAtomCue, Params);

			if (!ExistingComponent)
			{
				UE_LOG(LogMovieScene, Warning, TEXT("Failed to create atom component for spatialized atom track (row %d on %s)."), RowIndex, *ObjectName);
				return nullptr;
			}

			Player.SavePreAnimatedState(*ExistingComponent, FMovieSceneAnimTypeID::Unique(), FDestroyAtomPreAnimatedToken::FProducer());

			AtomComponentsByActorKey[ObjectKey].Add(SectionKey, ExistingComponent);

			ExistingComponent->SetFlags(RF_Transient);
			ExistingComponent->AttachToComponent(SceneComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}

		return ExistingComponent;
	}

	/** Only to be called on the game thread */
	UAtomComponent* AddRootAtomComponentForRow(int32 RowIndex, FObjectKey SectionKey, UWorld* World, IMovieScenePlayer& Player)
	{
		UAtomComponent* ExistingComponent = GetAtomComponent(FObjectKey(), SectionKey);
		if (!ExistingComponent)
		{
			UAtomSoundSimple* TempPlaybackAtomCue = NewObject<UAtomSoundSimple>();

			FAtomRuntime::FCreateComponentParams Params(World);

			ExistingComponent = FAtomRuntime::CreateComponent(TempPlaybackAtomCue, Params);

			if (!ExistingComponent)
			{
				UE_LOG(LogMovieScene, Warning, TEXT("Failed to create audio component for root audio track (row %d)."), RowIndex);
				return nullptr;
			}

			Player.SavePreAnimatedState(*ExistingComponent, FMovieSceneAnimTypeID::Unique(), FDestroyAtomPreAnimatedToken::FProducer());
			
			ExistingComponent->SetFlags(RF_Transient);

			AtomComponentsByActorKey[FObjectKey()].Add(SectionKey, ExistingComponent);
		}

		return ExistingComponent;
	}

	void StopAllSounds()
	{
		for (TPair<FObjectKey, TMap<FObjectKey, TWeakObjectPtr<UAtomComponent>>>& Map : AtomComponentsByActorKey)
		{
			for (TPair<FObjectKey, TWeakObjectPtr<UAtomComponent>>& Pair : Map.Value)
			{
				if (UAtomComponent* AtomComponent = Pair.Value.Get())
				{
					AtomComponent->Stop();
				}
			}
		}
	}
};

struct FAtomSectionExecutionToken : IMovieSceneExecutionToken
{
	FAtomSectionExecutionToken(const UMovieSceneAtomSection* InAtomSection)
		: AtomSection(InAtomSection), SectionKey(InAtomSection)
	{}

	virtual void Execute(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player)
	{
		FCachedAtomTrackData& TrackData = PersistentData.GetOrAddTrackData<FCachedAtomTrackData>();

		// If the status says we jumped, we always stop all sounds, then allow them to be played again naturally below if status == Playing (for example)
		if (Context.HasJumped())
		{
			TrackData.StopAllSounds();
		}

		if ((Context.GetStatus() != EMovieScenePlayerStatus::Playing && Context.GetStatus() != EMovieScenePlayerStatus::Scrubbing && Context.GetStatus() != EMovieScenePlayerStatus::Stepping) || Context.GetDirection() == EPlayDirection::Backwards)
		{
			// stopped, recording, etc
			TrackData.StopAllSounds();
		}

		// Root atom track
		else if (!Operand.ObjectBindingID.IsValid())
		{
			UObject* PlaybackContext = Player.GetPlaybackContext();

			const FMovieSceneActorReferenceData& AttachActorData = AtomSection->GetAttachActorData();

			USceneComponent* AttachComponent = nullptr;
			FMovieSceneActorReferenceKey AttachKey;
			AttachActorData.Evaluate(Context.GetTime(), AttachKey);
			FMovieSceneObjectBindingID AttachBindingID = AttachKey.Object;
			if (AttachBindingID.IsValid())
			{
				// If the transform is set, otherwise use the bound actor's transform
				for (TWeakObjectPtr<> WeakObject : AttachBindingID.ResolveBoundObjects(Operand.SequenceID, Player))
				{
					AActor* AttachActor = Cast<AActor>(WeakObject.Get());
					if (AttachActor)
					{
						AttachComponent = AtomSection->GetAttachComponent(AttachActor, AttachKey);
					}
					if (AttachComponent)
					{
						break;
					}
				}
			}

			UAtomComponent* AtomComponent = TrackData.GetAtomComponent(FObjectKey(), SectionKey);
			if (!AtomComponent)
			{
				// Initialize the sound
				AtomComponent = TrackData.AddRootAtomComponentForRow(AtomSection->GetRowIndex(), SectionKey, PlaybackContext ? PlaybackContext->GetWorld() : nullptr, Player);

				if (AtomComponent)
				{
					/*
					if (AtomSection->GetOnQueueSubtitles().IsBound())
					{
						AtomComponent->OnQueueSubtitles = AtomSection->GetOnQueueSubtitles();
					}
					*/
					if (AtomSection->GetOnAtomSoundFinished().IsBound())
					{
						AtomComponent->OnAtomSoundFinished = AtomSection->GetOnAtomSoundFinished();
					}
					if (AtomSection->GetOnAtomSoundPlaybackPercent().IsBound())
					{
						AtomComponent->OnAtomSoundPlaybackPercent = AtomSection->GetOnAtomSoundPlaybackPercent();
					}
				}
			}

			if (AtomComponent)
			{
				if (AttachComponent && (AtomComponent->GetAttachParent() != AttachComponent || AtomComponent->GetAttachSocketName() != AttachKey.SocketName))
				{
					AtomComponent->AttachToComponent(AttachComponent, FAttachmentTransformRules::KeepRelativeTransform, AttachKey.SocketName);
				}

				EnsureAtomIsPlaying(*AtomComponent, PersistentData, Context, AtomComponent->GetAttachParent() != nullptr, Player);
			}
		}

		// Object binding atom track
		else
		{
			for (TWeakObjectPtr<> Object : Player.FindBoundObjects(Operand))
			{
				UAtomComponent* AtomComponent = TrackData.GetAtomComponent(Object.Get(), SectionKey);
				if (!AtomComponent)
				{
					// Initialize the sound
					AtomComponent = TrackData.AddAtomComponentForRow(AtomSection->GetRowIndex(), SectionKey, *Object.Get(), Player);

					if (AtomComponent)
					{
						/*
						if (AtomSection->GetOnQueueSubtitles().IsBound())
						{
							AtomComponent->OnQueueSubtitles = AtomSection->GetOnQueueSubtitles();
						}
						*/
						if (AtomSection->GetOnAtomSoundFinished().IsBound())
						{
							AtomComponent->OnAtomSoundFinished = AtomSection->GetOnAtomSoundFinished();
						}
						if (AtomSection->GetOnAtomSoundPlaybackPercent().IsBound())
						{
							AtomComponent->OnAtomSoundPlaybackPercent = AtomSection->GetOnAtomSoundPlaybackPercent();
						}
					}
				}
				
				if (AtomComponent)
				{
					EnsureAtomIsPlaying(*AtomComponent, PersistentData, Context, true, Player);
				}
			}
		}
	}

	// Helper template to pair channel evaluation and parameter application.
	/*template<typename ChannelType, typename ValueType>
	void EvaluateAllAndSetParameters(IAudioParameterControllerInterface& InParamaterInterface, const FFrameTime& InTime) const
	{
		AtomSection->ForEachInput([&InParamaterInterface, &InTime](FName InName, const ChannelType& InChannel)
		{
			using namespace UE::MovieScene;
			ValueType OutValue{};
			if (EvaluateChannel(&InChannel, InTime, OutValue))
			{
				InParamaterInterface.SetParameter(InName, MoveTempIfPossible(OutValue));
			}
		});
	}

	void EvaluateAllAndFireTriggers(IAudioParameterControllerInterface& InParamaterInterface, const FMovieSceneContext& InContext, FCachedAtomTrackData& InPersistentData) const
	{
		AtomSection->ForEachInput([&InParamaterInterface, &InContext, &InPersistentData](FName InName, const FMovieSceneAudioTriggerChannel& InChannel)
		{
			bool OutValue = false;
			FMoveSceneAudioTriggerState& TriggerState = InPersistentData.TriggerStateMap.FindOrAdd(InName);
			if (InChannel.EvaluatePossibleTriggers(InContext, TriggerState, OutValue))
			{
				if (OutValue)
				{
					InParamaterInterface.SetTriggerParameter(InName);
				}
			}
		});
	}*/

	void EnsureAtomIsPlaying(UAtomComponent& AtomComponent, FPersistentEvaluationData& PersistentData, const FMovieSceneContext& Context, bool bAllowSpatialization, IMovieScenePlayer& Player) const
	{
		Player.SavePreAnimatedState(AtomComponent, FStopAtomPreAnimatedToken::GetAnimTypeID(), FStopAtomPreAnimatedToken::FProducer());

		float AudioVolume = 1.f;
		AtomSection->GetSoundVolumeChannel().Evaluate(Context.GetTime(), AudioVolume);
		AudioVolume = AudioVolume * AtomSection->EvaluateEasing(Context.GetTime());
		if (AtomComponent.VolumeMultiplier != AudioVolume)
		{
			AtomComponent.SetVolumeMultiplier(AudioVolume);
		}

		float PitchMultiplier = 1.f;
		AtomSection->GetPitchMultiplierChannel().Evaluate(Context.GetTime(), PitchMultiplier);
		if (AtomComponent.PitchMultiplier != PitchMultiplier)
		{
			AtomComponent.SetPitchMultiplier(PitchMultiplier);
		}

		//AtomComponent.bSuppressSubtitles = AtomSection->GetSuppressSubtitles();

		//Evaluate inputs and apply the params.
		//EvaluateAllAndSetParameters<FMovieSceneFloatChannel, float>(AtomComponent, Context.GetTime());
		//EvaluateAllAndSetParameters<FMovieSceneBoolChannel, bool>(AtomComponent, Context.GetTime());
		//EvaluateAllAndSetParameters<FMovieSceneIntegerChannel, int32>(AtomComponent, Context.GetTime());
		//EvaluateAllAndSetParameters<FMovieSceneStringChannel, FString>(AtomComponent, Context.GetTime());

		TArray<FScalarParameterNameAndCurve> ScalarParameterNamesAndCurves = AtomSection->GetScalarParameterNamesAndCurves();
		for (auto ScalarParameterNamesAndCurve : ScalarParameterNamesAndCurves)
		{
			float AisacValue = 0.0f;
			ScalarParameterNamesAndCurve.ParameterCurve.Evaluate(Context.GetTime(), AisacValue);

			FAtomAisacControl ControlParam;
			ControlParam.Name = ScalarParameterNamesAndCurve.ParameterName;
			ControlParam.ID = -1; // TODO id -> name function 

			AtomComponent.SetAisacControlValue(ControlParam, AisacValue);
		}

		float SectionStartTimeSeconds = (AtomSection->HasStartFrame() ? AtomSection->GetInclusiveStartFrame() : 0) / AtomSection->GetTypedOuter<UMovieScene>()->GetTickResolution();

		FCachedAtomTrackData& TrackData = PersistentData.GetOrAddTrackData<FCachedAtomTrackData>();
		const FFrameNumber AudioStartOffset = AtomSection->GetStartOffset();
		UAtomSoundBase* Sound = AtomSection->GetSound();

		float AudioTime = (Context.GetTime() / Context.GetFrameRate()) - SectionStartTimeSeconds + (float)Context.GetFrameRate().AsSeconds(AudioStartOffset);
		if (AudioTime >= 0.f && Sound)
		{
			const float Duration = Sound->GetDuration();
			//const float Duration = MovieSceneHelpers::GetSoundDuration(Sound);

			if (!AtomSection->GetLooping() && AudioTime > Duration && Duration != 0.f)
			{
				AtomComponent.Stop();
				return;
			}

			AudioTime = Duration > 0.f ? FMath::Fmod(AudioTime, Duration) : AudioTime;
		}

		// If the atom component is not playing we (may) need a state change. If the atom component is playing
		// the wrong sound then we need a state change. If the audio playback time is significantly out of sync 
		// with the desired time then we need a state change.
		bool bSoundNeedsStateChange = !AtomComponent.IsPlaying() || AtomComponent.Sound != Sound;
		bool bSoundNeedsTimeSync = false;

		UObject* PlaybackContext = Player.GetPlaybackContext();
		UWorld* World = PlaybackContext ? PlaybackContext->GetWorld() : nullptr;

		// Sync only if there is no time dilation because otherwise the system will constantly resync because audio 
		// playback is not dilated and will never match the expected playback time.
		const bool bDoTimeSync =
			World && World->GetWorldSettings() &&
			(FMath::IsNearlyEqual(World->GetWorldSettings()->GetEffectiveTimeDilation(), 1.f) ||
				!bIgnoreAtomSyncDuringWorldTimeDilationCVar);

		if (bDoTimeSync)
		{
			float CurrentGameTime = 0.0f;

			FAtomRuntime* AtomRuntime = World ? FAtomRuntimeManager::GetAtomRuntimeRawFromWorld(World) : nullptr;
			if (UseAtomClockForSequencerDesyncCVar && AtomRuntime)
			{
				CurrentGameTime = AtomRuntime->GetAtomClock();
			}
			else
			{
				CurrentGameTime = World ? World->GetAudioTimeSeconds() : 0.f;
			}

			// This tells us how much time has passed in the game world (and thus, reasonably, the audio playback)
			// so if we calculate that we should be playing say, 15s into the section during evaluation, but
			// we're only 5s since the last Play call, then we know we're out of sync. 
			if (TrackData.SoundLastPlayedAtTime.Contains(&AtomComponent))
			{
				float SoundLastPlayedAtTime = TrackData.SoundLastPlayedAtTime[&AtomComponent];

				float GameTimeDelta = CurrentGameTime - SoundLastPlayedAtTime;
				if (!FMath::IsNearlyZero(MaxSequenceAtomDesyncToleranceCVar) && FMath::Abs(GameTimeDelta - AudioTime) > MaxSequenceAtomDesyncToleranceCVar)
				{
					UE_LOG(LogMovieScene, Verbose, TEXT("Atom Component detected a significant mismatch in (assumed) playback time versus the desired time. Time since last play call: %6.2f(s) Desired Time: %6.2f(s). Component: %s sound: %s"), GameTimeDelta, AudioTime, *AtomComponent.GetName(), *GetNameSafe(AtomComponent.Sound));
					bSoundNeedsTimeSync = true;
				}
			}
		}

		if (bSoundNeedsStateChange || bSoundNeedsTimeSync)
		{
			AtomComponent.bAllowSpatialization = bAllowSpatialization;

			if (AtomSection->GetOverrideAttenuation())
			{
				AtomComponent.AttenuationSettings = AtomSection->GetAttenuationSettings();
				if (!AtomComponent.AttenuationSettings)
				{
					AtomComponent.AttenuationOverrides = AtomSection->GetAttenuationOverrides();
				}
			}


			// Call stop sound only if it's actually playing. 
			// This prevents the stop instruction from being called continuously when a sound cue with zero spawn time is playing.
			if (AtomComponent.IsPlaying() || bSoundNeedsTimeSync)
			{
				UE_LOG(LogMovieScene, Verbose, TEXT("Atom Component stopped due to needing a state change bIsPlaying: %d bNeedsTimeSync: %d. Component: %s sound: %s"), AtomComponent.IsPlaying(), bSoundNeedsTimeSync, *AtomComponent.GetName(), *AtomComponent.Sound->GetName());
				AtomComponent.Stop();
			}

			// Only change if the sound clip actually changed. Calls Stop internally when necessary.
			if (AtomComponent.Sound != Sound)
			{
				UE_LOG(LogMovieScene, Verbose, TEXT("Atom Component calling SetSound due to new sound. Component: %s OldSound: %s NewSound: %s"), *AtomComponent.GetName(), *GetNameSafe(AtomComponent.Sound), *AtomComponent.Sound->GetName());
				AtomComponent.SetSound(Sound);
			}
#if WITH_EDITOR
			if (GIsEditor && World != nullptr && !World->IsPlayInEditor())
			{
				AtomComponent.bIsUISound = true;
				AtomComponent.bIsPreviewSound = true;
			}
			else
#endif // WITH_EDITOR
			{
				AtomComponent.bIsUISound = false;
			}

			if (AudioTime >= 0.f)
			{
				AtomComponent.CueSelectors = AtomSection->GetSelectorLabels();
				AtomComponent.CueFirstBlockIndex = AtomSection->GetBlockIndex();

				UE_LOG(LogMovieScene, Verbose, TEXT("Atom Component Play at Local Time: %6.2f CurrentTime: %6.2f(s) SectionStart: %6.2f(s), SoundDur: %6.2f OffsetIntoClip: %6.2f sound: %s"), AudioTime, (Context.GetTime() / Context.GetFrameRate()), SectionStartTimeSeconds, AtomComponent.Sound->GetDuration(), (float)Context.GetFrameRate().AsSeconds(AudioStartOffset), *AtomComponent.Sound->GetName());
				AtomComponent.Play(AudioTime);

				// Keep track of when we asked this audio clip to play (in game time) so that we can figure out if there's a significant desync in the future.
				if (World)
				{
					if (!TrackData.SoundLastPlayedAtTime.Contains(&AtomComponent))
					{
						TrackData.SoundLastPlayedAtTime.Add(&AtomComponent);
					}

					FAtomRuntime* AtomRuntime = FAtomRuntimeManager::GetAtomRuntimeRawFromWorld(World);
					if (UseAtomClockForSequencerDesyncCVar && AtomRuntime)
					{
						TrackData.SoundLastPlayedAtTime[&AtomComponent] = AtomRuntime->GetAtomClock() - AudioTime;
					}
					else
					{
						TrackData.SoundLastPlayedAtTime[&AtomComponent] = World->GetAudioTimeSeconds() - AudioTime;
					}
				}
			}
		}

		if (Context.GetStatus() == EMovieScenePlayerStatus::Scrubbing || Context.GetStatus() == EMovieScenePlayerStatus::Stepping)
		{
			// While scrubbing, play the sound for a short time and then cut it.
			//AtomComponent.StopDelayed(AtomTrackConstants::ScrubDuration);
			AtomComponent.Stop();
		}

		/*if (AtomComponent.IsPlaying())
		{
			EvaluateAllAndFireTriggers(AtomComponent, Context, TrackData);
		}*/

		if (bAllowSpatialization)
		{
			if (FAtomRuntime* AtomRuntime = AtomComponent.GetAtomRuntime())
			{
				DECLARE_CYCLE_STAT(TEXT("FAtomThreadTask.MovieSceneUpdateAtomTransform"), STAT_MovieSceneUpdateAtomTransform, STATGROUP_TaskGraphTasks);
				AtomRuntime->SendCommandToActiveSounds(AtomComponent.GetAtomComponentID(), [ActorTransform = AtomComponent.GetComponentTransform()](FAtomActiveSound& ActiveSound)
				{
					ActiveSound.bLocationDefined = true;
					ActiveSound.Transform = ActorTransform;
				}, GET_STATID(STAT_MovieSceneUpdateAtomTransform));
			}
		}
	}

	const UMovieSceneAtomSection* AtomSection;
	FObjectKey SectionKey;
};

FMovieSceneAtomSectionTemplate::FMovieSceneAtomSectionTemplate()
	: AtomSection()
{
}

FMovieSceneAtomSectionTemplate::FMovieSceneAtomSectionTemplate(const UMovieSceneAtomSection& Section)
	: AtomSection(&Section)
{
}

void FMovieSceneAtomSectionTemplate::Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const
{
	MOVIESCENE_DETAILED_SCOPE_CYCLE_COUNTER(MovieSceneEval_AtomTrack_Evaluate)

	if (GEngine && GEngine->UseSound() && Context.GetStatus() != EMovieScenePlayerStatus::Jumping)
	{
		ExecutionTokens.Add(FAtomSectionExecutionToken(AtomSection));
	}
}
