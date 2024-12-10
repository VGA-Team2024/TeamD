/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCore
 * File     : AtomComponent.h
 *
 ****************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "UObject/ObjectMacros.h"
#include "Components/SceneComponent.h"
#include "Math/RandomStream.h"

#include "Atom.h"
#include "AtomAttenuation.h"
#include "AtomEnvelope.h"
#include "AtomSourceVoiceEffect.h"
#include "AtomParameter.h"
#include "AtomVolumeFader.h"
#include "Modulation/AtomModulationDestination.h"

#include "AtomComponent.generated.h"

 // Forward Declarations
class FAtomRuntime;
class UAtomComponent;
class UAtomSoundClass;
class UAtomConcurrency;
class UAtomSoundBase;
class UAtomCueSheet;
class UAtomSoundCue;
class UAtomWaveBank;
class UAtomSoundWave;
class UAtomRack;

// Enum describing the Atom component play state
UENUM(BlueprintType)
enum class EAtomComponentPlayState : uint8
{
	// If the sound is playing (i.e. not fading in, not fading out, not paused)
	Playing,

	// If the sound is not playing
	Stopped,

	// If the sound is playing but paused
	Paused,

	// If the sound is playing and fading in
	FadingIn,

	// If the sound is playing and fading out
	FadingOut,

	Count UMETA(Hidden)
};

/** called when we finish playing audio, either because it played to completion or because a Stop() call turned it off early */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAtomSoundFinished);

/** shadow delegate declaration for above */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAtomSoundFinishedNative, class UAtomComponent*);

/** Called when sound's PlayState changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAtomSoundPlayStateChanged, EAtomComponentPlayState, PlayState);

/** shadow delegate declaration for above */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAtomSoundPlayStateChangedNative, const UAtomComponent*, EAtomComponentPlayState);

/** Called when sound becomes virtualized or realized (resumes playback from virtualization). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAtomSoundVirtualizationChanged, bool, bIsVirtualized);

/** shadow delegate declaration for above */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAtomSoundVirtualizationChangedNative, const UAtomComponent*, bool);

/** Called as a sound plays on the Atom component to allow BP to perform actions based on playback percentage.
 * Computed as samples played divided by total samples, taking into account pitch.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAtomSoundPlaybackPercent, const class UAtomSoundBase*, PlayingSoundBase, const float, PlaybackPercent);

/** shadow delegate declaration for above */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnAtomSoundPlaybackPercentNative, const class UAtomComponent*, const class UAtomSoundBase*, const float);

/** Called when sound cue block index is reached. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAtomSoundCueBlockIndexChanged, int32, BlockIndex);

/** shadow delegate declaration for above */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAtomSoundCueBlockIndexChangedNative, const UAtomComponent*, int32);

/** Called when a beat occurs from cue sequence's BeatSync. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAtomSoundCueBeatSync, FAtomBeatSyncInfo, BeatSyncInfo);

/** shadow delegate declaration for above. */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAtomSoundCueBeatSyncNative, const UAtomComponent*, FAtomBeatSyncInfo);

/** Called when an event or a marker with a callback is reached in by cue sequencer. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAtomSoundCueSequencerEvent, FAtomSequencerEventInfo, EventInfo);

/** shadow delegate declaration for above */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAtomSoundCueSequencerEventNative, const UAtomComponent*, FAtomSequencerEventInfo);

/** Native delegate that is invoked when PCM data need to be filtered out.*/
//DECLARE_DELEGATE_FourParams(FOnFilterDataDelegate, int32, int32, FrameNumber, int32, NumChannels, int16*, Data);

/** Called by Atom when request cue sequence's BeatSync information from blueprint. */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAtomSoundCueBeatSyncInfo, FAtomBeatSyncInfo, BeatSyncInfo);

UENUM(BlueprintType)
enum class EAtomModulationDestination : uint8
{
	/* Volume modulation */
	Volume,

	/* Pitch modulation */
	Pitch,

	/* Cutoff Frequency of a lowpass filter */
	Lowpass,

	/* Cutoff Frequency of a highpass filter */
	Highpass,

	Count UMETA(Hidden)
};

/**
 * AtomComponent is used to play a Sound
 *
 * @see https://docs.unrealengine.com/WorkingWithAudio/Overview
 * @see UAtomSoundBase
 */
UCLASS(ClassGroup = (Atom), HideCategories = (Object, ActorComponent, Physics, Rendering, Mobility, LOD), ShowCategories = Trigger, meta = (BlueprintSpawnableComponent))
class CRIWARECORE_API UAtomComponent
	: public USceneComponent
	//, public IAtomParameterControllerInterface
	//, public FQuartzTickableObject
{
	GENERATED_BODY()

public:
	/** The sound asset to be played. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	TObjectPtr<UAtomSoundBase> Sound;

	/** Array of parameters for this AtomComponent. Changes to this array directly will
	  * not be forwarded to the sound if the component is actively playing, and will be superseded
	  * by parameters set via the actor interface if set, or the instance parameters.
	  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Parameters, meta = (DisplayAfter = "bDisableParameterUpdatesWhilePlaying"))
	TArray<FAtomParameter> DefaultParameters;

	/** Array of transient parameters for this AtomComponent instance. Not serialized and can be set by code or BP.
	  * Changes to this array directly will not be forwarded to the sound if the component is actively playing.
	  * This should be done via the 'SetParameterX' calls implemented by the IAtomParameterControllerInterface.
	  * Instance parameter values superseded the parameters set by the actor interface & the component's default
	  * parameters.
	  */
	UPROPERTY(Transient)
	TArray<FAtomParameter> InstanceParameters;

	/** SoundClass that overrides that set on the referenced SoundBase when component is played. */
	UPROPERTY(EditAnywhere, Category = Sound, AdvancedDisplay)
	TObjectPtr<UAtomSoundClass> SoundClassOverride;

	/** Auto destroy this component on completion */
	UPROPERTY()
	uint8 bAutoDestroy : 1;

	/** Stop sound when owner is destroyed */
	UPROPERTY()
	uint8 bStopWhenOwnerDestroyed : 1;

	/** Whether the plaback instances should remain active if they're dropped by the prioritization code. Useful for e.g. vehicle sounds that shouldn't cut out. */
	UPROPERTY()
	uint8 bShouldRemainActiveIfDropped : 1;

	/** Overrides spatialization enablement in either the attenuation asset or on this Atom component's attenuation settings override. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attenuation)
	uint8 bAllowSpatialization : 1;
    
    /** Allows defining attenuation settings directly on this audio component without using an attenuation settings asset. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetOverrideAttenuation, Category = Attenuation)
    uint8 bOverrideAttenuation : 1;

    UFUNCTION(BlueprintSetter)
    void SetOverrideAttenuation(bool bInOverrideAttenuation);

	/** Whether or not this sound plays when the game is paused in the UI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, AdvancedDisplay)
	uint8 bIsUISound : 1;

	/** Whether or not to apply a low-pass filter to the sound that plays in this Atom component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, meta = (InlineEditConditionToggle, DisplayAfter = "PitchMultiplier"))
	uint8 bEnableLowPassFilter : 1;

	/** Whether or not to apply an amplitude envelope to the sound that plays in this Atom component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, meta = (InlineEditConditionToggle, DisplayAfter = "LowPassFilterFrequency"))
	uint8 bEnableAmplitudeEnvelope : 1;

	/** Whether or not to override the priority of the given sound with the value provided. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, meta = (InlineEditConditionToggle, DisplayAfter = "AmplitudeEnvelope"))
	uint8 bOverridePriority : 1;

	/** If true, the Atom Component will play multiple sound instances at once. Switching sounds or calling play while already playing
	  * will not stop already active instances. Disabling while sound(s) are playing
	  * will not take effect until the AtomComponent is stopped and restarted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, meta = (DisplayName = "Play Multiple Instances", DisplayAfter = "Priority"))
	uint8 bCanPlayMultipleInstances : 1;

	/** If true, the Audio Component will ignore parameter updates for already-playing sound(s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Parameters)
	uint8 bDisableParameterUpdatesWhilePlaying : 1;

	/** Whether this Atom component is previewing a sound */
	uint8 bPreviewComponent : 1;

	/** Whether or not this sound is a preview sound */
	uint8 bIsPreviewSound : 1;

	/** If true, this sound will not be stopped when flushing the Atom runtime. */
	uint8 bIgnoreForFlushing : 1;

	/** Whether to artificially prioritize the component to play */
	uint8 bAlwaysPlay : 1;

	/** Whether or not this audio component is a music clip */
	//uint8 bIsMusic : 1;

	/** Whether or not the audio component should be excluded from reverb EQ processing */
	//uint8 bReverb : 1;

	/** Whether or not this sound class forces sounds to the center channel */
	//uint8 bCenterChannelOnly : 1;

	/** Whether or not this Atom component has been paused */
	uint8 bIsPaused : 1;

	/** Whether or not this Atom component's sound is virtualized */
	uint8 bIsVirtualized : 1;

	/** Whether or not fade out was triggered. */
	uint8 bIsFadingOut : 1;

	/**
	 * True if we should automatically attach to AutoAttachParent when Played, and detach from our parent when playback is completed.
	 * This overrides any current attachment that may be present at the time of activation (deferring initial attachment until activation, if AutoAttachParent is null).
	 * If enabled, this AtomComponent's WorldLocation will no longer be reliable when not currently playing audio, and any attach children will also be
	 * detached/attached along with it.
	 * When enabled, detachment occurs regardless of whether AutoAttachParent is assigned, and the relative transform from the time of activation is restored.
	 * This also disables attachment on dedicated servers, where we don't actually activate even if bAutoActivate is true.
	 * @see AutoAttachParent, AutoAttachSocketName, AutoAttachLocationType
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attachment)
	uint8 bAutoManageAttachment : 1;

private:

	/** Did we auto attach during activation? Used to determine if we should restore the relative transform during detachment. */
	uint8 bDidAutoAttach : 1;

public:

	/** The specific Atom runtime to play this component on */
	uint32 AtomRuntimeID;

	/** Configurable, serialized ID for plugins */
	UPROPERTY()
	FName AtomComponentUserID;

	/** The lower bound to use when randomly determining a pitch multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Randomization|Pitch", meta = (DisplayName = "Pitch (Min)"))
	float PitchModulationMin;

	/** The upper bound to use when randomly determining a pitch multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Randomization|Pitch", meta = (DisplayName = "Pitch (Max)"))
	float PitchModulationMax;

	/** The lower bound to use when randomly determining a volume multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Randomization|Volume", meta = (DisplayName = "Volume (Min)"))
	float VolumeModulationMin;

	/** The upper bound to use when randomly determining a volume multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Randomization|Volume", meta = (DisplayName = "Volume (Max)"))
	float VolumeModulationMax;

	/** A volume multiplier to apply to sounds generated by this component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, meta = (DisplayAfter = "Sound"))
	float VolumeMultiplier;

	/** A pitch multiplier to apply to sounds generated by this component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, meta = (DisplayAfter = "VolumeMultiplier"))
	float PitchMultiplier;

	/** If enabled, the frequency of the Lowpass Filter (in Hz) to apply to this voice. A frequency of 0.0 is the device sample rate and will bypass the filter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "bEnableLowPassFilter", DisplayAfter = "PitchMultiplier"))
	float LowPassFilterFrequency;

	/** If enabled, override the envelope parameters to apply to voices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, meta = (EditCondition = "bEnableAmplitudeEnvelope", DisplayAfter = "LowPassFilterFrequency"))
	FAtomEnvelope AmplitudeEnvelope;

	/** Selects a scaling effect (Pitch Shifter or Time Stretch) to apply to the source voices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Effects", meta = (DisplayName = "Effect Type"))
	EAtomSourceVoiceEffectType SourceVoiceEffectType;

	/** If Pitch Shifter is enabled, the parameters to use to resample and scale the source voices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Effects", meta = (EditCondition = "SourceVoiceEffectType == EAtomSourceVoiceEffectType::PitchShifter", EditConditionHides))
	FAtomSourceEffectPitchShifterSettings PitchShifter;

	/** If Time Stretch is enabled, the parameters to use to resample and scale the source voices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Effects", meta = (EditCondition = "SourceVoiceEffectType == EAtomSourceVoiceEffectType::TimeStretch", EditConditionHides))
	FAtomSourceEffectTimeStretchSettings TimeStretch;

	/** The attack time in milliseconds for the envelope follower. Delegate callbacks can be registered to get the 
	 *  envelope value of sounds played with this audio component.
	 */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Analysis, meta = (ClampMin = "0", UIMin = "0"))
	//int32 EnvelopeFollowerAttackTime;

	/** The release time in milliseconds for the envelope follower. Delegate callbacks can be registered to get the
	 *  envelope value of sounds played with this audio component.
	 */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Analysis, meta = (ClampMin = "0", UIMin = "0"))
	//int32 EnvelopeFollowerReleaseTime;

	/** A count of how many times we've started playing */
	int32 ActiveCount;

	/** If enabled, overrides the priority of the selected sound with the value provided. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "bOverridePriority", DisplayAfter = "AmplitudeEnvelope"))
	float Priority;

	/** If bOverrideSettings is false, the asset to use to determine attenuation properties for sounds generated by this component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attenuation, BlueprintSetter = SetAttenuationSettings, meta = (EditCondition = "!bOverrideAttenuation", DisplayAfter = "bOverrideAttenuation", EditConditionHides))
	TObjectPtr<UAtomAttenuation> AttenuationSettings;
    
    UFUNCTION(BlueprintSetter)
    void SetAttenuationSettings(UAtomAttenuation* InAttenuationSettings);

    /** If bOverrideSettings is true, the attenuation properties to use for sounds generated by this component */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attenuation, BlueprintSetter = SetAttenuationOverrides, meta = (EditCondition = "bOverrideAttenuation", DisplayAfter = "bOverrideAttenuation", EditConditionHides))
    struct FAtomAttenuationSettings AttenuationOverrides;

    UFUNCTION(BlueprintSetter)
    void SetAttenuationOverrides(const FAtomAttenuationSettings& InAttenuationOverrides);

	/** While playing, this component will check for occlusion from its closest listener every this many seconds */
	float OcclusionCheckInterval;

	/** What time the player was told to play. Used to compute state. */
	float TimeAtomComponentPlayed;

	/** How much time the player was told to fade in. */
	float FadeInTimeDuration;

	/** */
	int32 CueBlockIndex;

	/** */
	FAtomBeatSyncInfo CueBeatSyncInfo;

	/** What sound concurrency rules to use for sounds generated by this Atom component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Concurrency)
	TSet<TObjectPtr<UAtomConcurrency>> ConcurrencySet;

	/**
	 * Options for how we handle our location when we attach to the AutoAttachParent, if bAutoManageAttachment is true.
	 * @see bAutoManageAttachment, EAttachmentRule
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attachment, meta = (EditCondition = "bAutoManageAttachment"))
	EAttachmentRule AutoAttachLocationRule;

	/**
	 * Options for how we handle our rotation when we attach to the AutoAttachParent, if bAutoManageAttachment is true.
	 * @see bAutoManageAttachment, EAttachmentRule
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attachment, meta = (EditCondition = "bAutoManageAttachment"))
	EAttachmentRule AutoAttachRotationRule;

	/**
	 * Options for how we handle our scale when we attach to the AutoAttachParent, if bAutoManageAttachment is true.
	 * @see bAutoManageAttachment, EAttachmentRule
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attachment, meta = (EditCondition = "bAutoManageAttachment"))
	EAttachmentRule AutoAttachScaleRule;

	/**
	 * Component we automatically attach to when activated, if bAutoManageAttachment is true.
	 * If null during registration, we assign the existing AttachParent and defer attachment until we activate.
	 * @see bAutoManageAttachment
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = Attachment, meta = (EditCondition = "bAutoManageAttachment"))
	TWeakObjectPtr<USceneComponent> AutoAttachParent;

	/**
	 * Socket we automatically attach to on the AutoAttachParent, if bAutoManageAttachment is true.
	 * @see bAutoManageAttachment
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attachment, meta = (EditCondition = "bAutoManageAttachment"))
	FName AutoAttachSocketName;

	/* Modulation for sound parameters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Modulation)
	FAtomSoundModulationRoutingSettings ModulationRouting;

	/** When sound is a SoundCue, this defines the selectors to use. Possible selector name and labels are defined by the currently used AtomConfig. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SoundCue, DisplayName = "Selectors")
	TArray<FAtomSelectorParam> CueSelectors;

	/** When sound is a SoundCue, this defines the index of the first block to playback at start. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SoundCue, DisplayName = "First Block Index")
	int32 CueFirstBlockIndex;

	/** Called when we finish playing sound, either because it played to completion or because a Stop() call turned it off early */
	UPROPERTY(BlueprintAssignable)
	FOnAtomSoundFinished OnAtomSoundFinished;

	/** Shadow delegate for non UObject subscribers. */
	FOnAtomSoundFinishedNative OnAtomSoundFinishedNative;

	/** This function returns the Targeted Atom Component's current Play State.
	  * Playing, if the sound is currently playing.
	  * Stopped, if the sound is stopped.
	  * Paused, if the sound is currently playing, but paused.
	  * Fading In, if the sound is in the process of Fading In.
	  * Fading Out, if the sound is in the process of Fading Out.
	  */
	UPROPERTY(BlueprintAssignable)
	FOnAtomSoundPlayStateChanged OnAtomSoundPlayStateChanged;

	/** Shadow delegate for non UObject subscribers. */
	FOnAtomSoundPlayStateChangedNative OnAtomSoundPlayStateChangedNative;

	/** Called when virtualization state changes. */
	UPROPERTY(BlueprintAssignable)
	FOnAtomSoundVirtualizationChanged OnAtomSoundVirtualizationChanged;

	/** Shadow delegate for non UObject subscribers. */
	FOnAtomSoundVirtualizationChangedNative OnAtomSoundVirtualizationChangedNative;

	/** Called as a sound plays on the Atom component to allow BP to perform actions based on playback percentage.
	 *  Computed as samples played divided by total samples, taking into account pitch.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnAtomSoundPlaybackPercent OnAtomSoundPlaybackPercent;

	/** Shadow delegate for non UObject subscribers. */
	FOnAtomSoundPlaybackPercentNative OnAtomSoundPlaybackPercentNative;

	/** Called when sound is a cue and a block index is reached. */
	UPROPERTY(BlueprintAssignable)
	FOnAtomSoundCueBlockIndexChanged OnAtomSoundCueBlockIndexChanged;

	/** Shadow delegate for non UObject subscribers. */
	FOnAtomSoundCueBlockIndexChangedNative OnAtomSoundCueBlockIndexChangedNative;

	/** Called when  . */
	UPROPERTY(BlueprintAssignable)
	FOnAtomSoundCueBeatSync OnAtomSoundCueBeatSync;

	/** Shadow delegate for non UObject subscribers. */
	FOnAtomSoundCueBeatSyncNative OnAtomSoundCueBeatSyncNative;

	/** Called when  . */
	UPROPERTY(BlueprintAssignable)
	FOnAtomSoundCueSequencerEvent OnAtomSoundCueSequencerEvent;

	/** Shadow delegate for non UObject subscribers. */
	FOnAtomSoundCueSequencerEventNative OnAtomSoundCueSequencerEventNative;

public:

	// Set what sound is played by this component
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void SetSound(UAtomSoundBase* NewSound);

	/**
	 * This function allows designers to call Play on an AtomComponent instance while applying a volume curve over time.
	 * Parameters allow designers to indicate the duration of the fade, the curve shape, and the start time if seeking into the sound.
	 *
	 * @param FadeInDuration How long it should take to reach the FadeVolumeLevel
	 * @param FadeVolumeLevel The percentage of the AtomComponents's calculated volume to fade to
	 * @param FadeCurve The curve to use when interpolating between the old and new volume
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound", meta = (AdvancedDisplay = 1))
	virtual void FadeIn(float FadeInDuration, float FadeVolumeLevel = 1.0f, float StartTime = 0.0f , const EAtomFaderCurve FadeCurve = EAtomFaderCurve::Linear);

	/**
	 * This function allows designers to call a delayed Stop on an Atom Component instance while applying a
	 * volume curve over time. Parameters allow designers to indicate the duration of the fade and the curve shape.
	 *
	 * @param FadeOutDuration how long it should take to reach the FadeVolumeLevel
	 * @param FadeVolumeLevel the percentage of the AtomComponents's calculated volume in which to fade to
	 * @param FadeCurve The curve to use when interpolating between the old and new volume
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound", meta = (AdvancedDisplay = 1))
	virtual	void FadeOut(float FadeOutDuration, float FadeVolumeLevel, const EAtomFaderCurve FadeCurve = EAtomFaderCurve::Linear);

	/** Begins playing the targeted Atom Component's sound at the designated Start Time, seeking into a sound.
	 * @param StartTime The offset, in seconds, to begin reading the sound at
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	virtual void Play(float StartTime = 0.0f);

	/** Start a sound playing on an Atom component on a given quantization boundary with the handle to an existing clock from Unreal Quartz */
	/*UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "3", UnsafeDuringActorConstruction = "true", Keywords = "play", AutoCreateRefTerm = "InDelegate"))
	virtual void PlayQuantized(
		const UObject* WorldContextObject
		, UPARAM(ref) UQuartzClockHandle*& InClockHandle
		, UPARAM(ref) FQuartzQuantizationBoundary& InQuantizationBoundary
		, const FOnQuartzCommandEventBP& InDelegate
		, float InStartTime = 0.f
		, float InFadeInDuration = 0.f
		, float InFadeVolumeLevel = 1.f
		//, EAtomFaderCurve InFadeCurve = EAtomFaderCurve::Linear
	);*/

	//For if this is being played through a sound queued through Quartz
	//virtual void PlayQueuedQuantizedInternal(const UObject* WorldContextObject, FAudioComponentCommandInfo InCommandInfo);

	// Sets a named Boolean
	/*UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Boolean Parameter"), Category = "Audio|Parameter")
		virtual void SetBoolParameter(FName InName, bool InBool) override
	{
		return ISoundParameterControllerInterface::SetBoolParameter(InName, InBool);
	}

	// Sets a named Int32
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Integer Parameter"), Category = "Audio|Parameter")
		virtual void SetIntParameter(FName InName, int32 InInt) override
	{
		return ISoundParameterControllerInterface::SetIntParameter(InName, InInt);
	}

	// Sets a named Float
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Float Parameter"), Category = "Audio|Parameter")
		virtual void SetFloatParameter(FName InName, float InFloat) override
	{
		return ISoundParameterControllerInterface::SetFloatParameter(InName, InFloat);
	}

	virtual void ResetParameters() override;*/
	void ResetParameters();
	
	/** Stop sound, issue any delegates if needed */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void Stop();

	/** Cues request to stop sound after the provided delay, stopping immediately if delay is zero or negative */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void StopDelayed(float DelayTime);

	/** Pause sound playing, issue any delegates if needed */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void SetPaused(bool bPause = true);

	/** Returns true if the targeted AtomComponent's sound is playing.
	 *  Doesn't indicate if the sound is paused or fading in/out. Use GetPlayState() to get the full play state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	virtual bool IsPlaying() const;

	/** Returns if the sound is virtualized. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	bool IsVirtualized() const;

	/** Returns the enumerated play states of the Atom component. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	EAtomComponentPlayState GetPlayState() const;

	/** This function allows designers to trigger an adjustment to the sound instance's playback Volume with options for smoothly applying a curve over time.
	 * @param AdjustVolumeDuration The length of time in which to interpolate between the initial volume and the new volume.
	 * @param AdjustVolumeLevel The new volume to set the AtomComponent to.
	 * @param FadeCurve The curve used when interpolating between the old and new volume.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void AdjustVolume(float AdjustVolumeDuration, float AdjustVolumeLevel, const EAtomFaderCurve FadeCurve = EAtomFaderCurve::Linear);

	/** Set a new volume multiplier */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void SetVolumeMultiplier(float NewVolumeMultiplier);

	/** Set a new pitch multiplier */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void SetPitchMultiplier(float NewPitchMultiplier);

	/** When set to TRUE, enables an additional Low Pass Filter Frequency to be calculated in with the
	 *  sound instance's LPF total, allowing designers to set filter settings for the targeted Atom Component's
	 *  sound instance.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void SetLowPassFilterEnabled(bool InLowPassFilterEnabled);

	/** Sets a cutoff frequency, in Hz, for the targeted Atom Component's sound's Low Pass Filter calculation.
	 *  The lowest cutoff frequency from all of the sound instance's possible LPF calculations wins.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void SetLowPassFilterFrequency(float InLowPassFilterFrequency);

	/** Sets an amplitude envelope to this Atom compoment that will be applied to sounds to play. 
	 * This overrides any amplitude envelope parameters set in cue data if present.
	 * It is worth noting that Envelope parameters are only passed to new Active Sounds on start, so modified Envelope data should be set before sound playback. 
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void SetAmplitudeEnvelope(FAtomEnvelope Envelope);

	/** Removes a previously set amplitude envelope to this Atom compoment.
	 * This reset to amplitude envelope parameters set in cue data if present.
	 * It is worth noting that Envelope parameters are only passed to new Active Sounds on start, so Envelope data should be removed before sound playback.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void ClearAmplitudeEnvelope();

	/** Sets whether sounds generated by this Atom component should be considered UI sounds */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void SetUISound(bool bInUISound);

	/** Enables a voice effect to apply to the source voices.
	 * Disabled by default, the parameters in respective settings of selected effect will apply when enabled.
	 * If Pitch Shifter effect is set, use SetPitchShifterPitch() and SetPitchShifterFormant() to adjust pitch and formant of the effect in real time.
	 * If Time Stretch effect is set, use SetTimeStretchRatio() to adjust the stretch ratio of the effect in real time.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void SetSourceVoiceEffectType(EAtomSourceVoiceEffectType VoiceEffecType);

	/** Sets the pitch shift in semitones of the Pitch Shifter effect if enabled. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void SetPitchShifterPitch(float Semitones);

	/** Sets the formant shift in semitones of the Pitch Shifter effect if enabled. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void SetPitchShifterFormant(float Semitones);

	/** Sets the time ratio of the Time Stretch effect if enabled. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void SetTimeStretchRatio(float Ratio);

	/** This function is used to modify the Attenuation Settings on the targeted Atom Component instance. 
	 * It is worth noting that Attenuation Settings are only passed to new Active Sounds on start, so modified Attenuation data should be set before sound playback. 
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void AdjustAttenuation(const FAtomAttenuationSettings& InAttenuationSettings);

	/** Allows designers to target a specific Atom Component instance's sound set the send level (volume of sound copied) to the indicated AtomRack.
	 * @param Rack The AtomRack to send the signal to.
	 * @param SendLevel The scalar used to alter the volume of the copied signal. (Not usable yet, always 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void SetRackSend(UAtomRackBase* Rack/*, float SendLevel*/);

	/** Sets how much audio the sound should send to the given Atom Bus (PRE Source Effects).
	 *  if the Bus Send doesn't already exist, it will be added to the overrides on the active sound. 
	 * @param AtomBus The Bus to send the signal to
	 * @param AtomBusSendLevel The scalar used to alter the volume of the copied signal
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void SetAtomBusSendPreEffect(UAtomBus* AtomBus, float AtomBusSendLevel);

	/** Sets how much audio the sound should send to the given Atom Bus (POST Source Effects).
	 *  if the Audio Bus Send doesn't already exist, it will be added to the overrides on the active sound. 
	 * @param AtomBus The Bus to send the signal to
	 * @param AtomBusSendLevel The scalar used to alter the volume of the copied signal
	 */
	//UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	//void SetAtomBusSendPostEffect(UAtomBus* AtomBus, float AtomBusSendLevel);

	/**
	 * Sets the routing for one of the given Audio component's Modulation Destinations.
	 * @param Modulators The set of modulators to apply to the given destination on the component.
	 * @param Destination The destination to assign the modulators to.
	 * @param RoutingMethod The routing method to use for the given modulator.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Components|Audio", DisplayName = "Set Modulation Routing")
	void SetModulationRouting(const TSet<UAtomModulatorBase*>& Modulators, const EAtomModulationDestination Destination, const EAtomModulationRouting RoutingMethod = EAtomModulationRouting::Inherit);

	/**
	 * Gets the set of currently active modulators for a given Modulation Destination.
	 * @param Destination The Destination to retrieve the Modulators from.
	 * @return The set of of Modulators applied to this component for the given Destination.
	 */
	UFUNCTION(BlueprintPure, Category = "Audio|Components|Audio", DisplayName = "Get Modulators")
	UPARAM(DisplayName = "Modulators") TSet<UAtomModulatorBase*> GetModulators(const EAtomModulationDestination Destination);

	/**
	 * Sets multiple source attenuation positions if spatialization is enabled.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void SetMultiplePositions(TArray<FTransform> Positions, EAtomMultiPositionType MultiPositionType);

	/** Modulates in realtime the desired AISAC control value.
	 * This multiply with the base control value and with any routed modulation.
	 * @param AisacControl The AISAC control to modulate.
	 * @param Value The value used to modulate the base value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound")
	void SetAisacControlValue(FAtomAisacControl AisacControl, float Value);

	/** */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound|Cue")
	void SetCueSelectorLabel(FName SelectorName, FName SelectorLabel);

	/** */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound|Cue")
	void RemoveCueSelectorLabel(FName SelectorName);

	/** */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound|Cue")
	void ClearAllCueSelectorLabels();

	/** */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound|Cue")
	void SetCueNextBlockIndex(int32 BlockIndex);

	/** */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound|Cue")
	int32 GetCueBlockIndex() const;

	/** Gets the current cue's BeatSync information. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound|Cue")
	const FAtomBeatSyncInfo& GetCueBeatSyncInfo() const;

	/** Sets the cue's BeatSync time offset in milliseconds. */
	UFUNCTION(BlueprintCallable, Category = "Atom|Components|Sound|Cue")
	void SetCueBeatSyncOffset(int32 TimeOffset);

	static void PlaybackCompleted(uint64 AtomComponentID, bool bFailedToStart);

private:

	/** Called by the ActiveSound to inform the component that playback is finished */
	void PlaybackCompleted(bool bFailedToStart);

	/** Whether or not the sound is audible. */
	bool IsInAudibleRange(float* OutMaxDistance) const;

	struct FAtomSoundPlaybackTimeData
	{
		USoundBase* Sound;
		float PlaybackTime;

		// Cached indices to boost searching cooked data indices
		uint32 LastEnvelopeCookedIndex;
		uint32 LastFFTCookedIndex;

		FAtomSoundPlaybackTimeData()
			: Sound(nullptr)
			, PlaybackTime(0.0f)
			, LastEnvelopeCookedIndex(INDEX_NONE)
			, LastFFTCookedIndex(INDEX_NONE)
		{}

		FAtomSoundPlaybackTimeData(USoundBase* InSound)
			: Sound(InSound)
			, PlaybackTime(0.0f)
			, LastEnvelopeCookedIndex(INDEX_NONE)
			, LastFFTCookedIndex(INDEX_NONE)
		{}
	};
	// The current playback times of sound playbacks in this Atom component.
	TMap<uint32, FAtomSoundPlaybackTimeData> SoundPlaybackTimes;

	void CancelAutoAttachment(bool bDetachFromParent, const UWorld* MyWorld);

	void SetBusSendEffectInternal(UAtomBus* InAtomBus, float SendLevel);

	void BroadcastPlayState();

	/** Returns the owning world's "AudioTime" - affected by world pause, but not time dilation.  If no world exists, returns the application time */
	float GetAudioTimeSeconds() const;

public:

	/** Set when the sound is finished with initial fading in */
	void SetFadeInComplete();

	/** Sets whether or not sound instance is virtualized */
	void SetIsVirtualized(bool bInIsVirtualized);

	/** Set the new current bloc index. Valid when sound cue is used.  */
	void SetCueBlockIndex(int32 BlockIndex);

	/** */
	void SetCueBeatSyncInfo(const FAtomBeatSyncInfo& BeatSyncInfo);

	/** */
	void OnCueBeatSync(const FAtomBeatSyncInfo& BeatSyncInfo);

	/** */
	void OnCueSequencerEvent(const FAtomSequencerEventInfo& EventInfo);

public:

	//~ Begin UObject Interface.
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	virtual FString GetDetailedInfoInternal() const override;
	virtual FName GetFNameForStatID() const override;
	virtual void PostLoad() override;
	virtual void Serialize(FArchive& Ar) override;
	virtual void BeginDestroy() override;
	//~ End UObject Interface.

	//~ Begin USceneComponent Interface
	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None) override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ End USceneComponent Interface

	//~ Begin ActorComponent Interface.
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual const UObject* AdditionalStatObject() const override;
	virtual bool IsReadyForOwnerToAutoDestroy() const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End ActorComponent Interface.

public:
	UAtomComponent(const FObjectInitializer& ObjectInitializer);

	uint64 GetAtomComponentID() const { return AtomComponentID; }

	FName GetAtomComponentUserID() const { return AtomComponentUserID; }

	static UAtomComponent* GetAtomComponentFromID(uint64 AtomComponentID);

	FAtomRuntime* GetAtomRuntime() const;
	//TArray<FAudioParameter>& GetInstanceParameters() override { return InstanceParameters; }
	//uint64 GetInstanceOwnerID() const override { return AtomComponentID; }
	UAtomSoundBase* GetSound() { return Sound; }

	/** Collects the various attenuation shapes that may be applied to the sound played 
	 * by the Atom component for visualization in the editor 
	 * or via the in game debug visualization. */
	void CollectAttenuationShapesForVisualization(TMultiMap<EAtomAttenuationShape, FAtomAttenuationSettings::AttenuationShapeDetails>& ShapeDetailsMap) const;

	// Sets the Atom thread playback time as used by the active sound playing this Atom component
	// Will be set if the Atom component is using baked FFT or envelope following data so as to be able to feed that data to BP based on playback time
	void SetPlaybackTimes(const TMap<uint32, float>& InSoundPlaybackTimes);

private:

	uint64 AtomComponentID;

	float RetriggerTimeSinceLastUpdate;
	float RetriggerUpdateInterval;

	/** Saved relative transform before auto attachment. Used during detachment to restore the transform if we had automatically attached. */
	FVector SavedAutoAttachRelativeLocation;
	FRotator SavedAutoAttachRelativeRotation;
	FVector SavedAutoAttachRelativeScale3D;

	struct PlayInternalRequestData
	{
		// start time
		float StartTime = 0.0f;

		// fade data
		float FadeInDuration = 0.0f;
		float FadeVolumeLevel = 1.0f;
		EAtomFaderCurve FadeCurve = EAtomFaderCurve::Linear;

		// Quantized event data
		//Audio::FQuartzQuantizedRequestData QuantizedRequestData;
	};

	static uint64 AtomComponentIDCounter;
	static TMap<uint64, UAtomComponent*> AtomIDToComponentMap;
	static FCriticalSection AtomIDToComponentMapLock;

protected:

	/** Utility function called by Play and FadeIn to start a sound playing. */
	void PlayInternal(const PlayInternalRequestData& InPlayRequestData, UAtomSoundBase* InSoundOverride = nullptr);

	void AdjustVolumeInternal(float AdjustVolumeDuration, float AdjustVolumeLevel, bool bIsFadeOut, EAtomFaderCurve FadeCurve);

	/** Returns a pointer to the attenuation settings to be used (if any) for this Atom component dependent on the AtomAttenuation asset or overrides set. */
	const FAtomAttenuationSettings* GetAttenuationSettingsToApply() const;
    const TObjectPtr<UAtomAttenuation> GetAttenuationSettingsAsset() const;
	bool GetOverrideAttenuation() const;

	/** Retrieves Attenuation Settings data on the targeted Atom Component. Returns false if no settings were found.
	 *  Because the Attenuation Settings data structure is copied, false returns will return default values.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Components|Sound", meta = (DisplayName = "Get Attenuation Settings To Apply", ScriptName = "GetAttenuationSettingsToApply"))
	bool BP_GetAttenuationSettingsToApply(FAtomAttenuationSettings& OutAttenuationSettings);

#if WITH_EDITORONLY_DATA
	/** Utility function that updates which texture is displayed on the sprite dependent on the properties of the Atom Component. */
	void UpdateSpriteTexture();
#endif

	FRandomStream RandomStream;
};
