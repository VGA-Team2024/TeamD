/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : CriWareMovieScene
 * File     : MovieSceneAtomSection.h
 *
 ****************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "MovieSceneSection.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneStringChannel.h"
#include "Channels/MovieSceneBoolChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"
#include "Sections/MovieSceneActorReferenceSection.h"
#include "Sections/MovieSceneParameterSection.h"
#include "Channels/MovieSceneAudioTriggerChannel.h"

#include "Atom/AtomComponent.h"

#include "MovieSceneAtomSection.generated.h"

// Forward Definitions
class UAtomSoundBase;
//class UAtomComponent;
class UAtomAttenuation;
class UAtomRack;
class UAtomBus;

/**
 * Atom section, for use in the master audio, or by attached audio objects
 */
UCLASS()
class CRIWAREMOVIESCENES_API UMovieSceneAtomSection
	: public UMovieSceneSection
{
	GENERATED_BODY()

public:

	UMovieSceneAtomSection(const FObjectInitializer& ObjectInitializer);

	/** Sets this section's sound */
	UFUNCTION(BlueprintCallable, Category = "Sequencer|Section")
	void SetSound(class UAtomSoundBase* InSound);

	/** Gets the sound for this section */
	UFUNCTION(BlueprintPure, Category = "Sequencer|Section")
	class UAtomSoundBase* GetSound() const { return Sound; }

	/** Set the offset into the beginning of the audio clip */
	UFUNCTION(BlueprintCallable, Category = "Sequencer|Section")
	void SetStartOffset(FFrameNumber InStartOffset) { StartFrameOffset = InStartOffset;}

	/** Get the offset into the beginning of the audio clip */
	UFUNCTION(BlueprintPure, Category = "Sequencer|Section")
	FFrameNumber GetStartOffset() const {return StartFrameOffset;}

	/**
	 * Gets the sound volume curve
	 *
	 * @return The rich curve for this sound volume
	 */
	const FMovieSceneFloatChannel& GetSoundVolumeChannel() const { return SoundVolume; }

	/**
	 * Gets the sound pitch curve
	 *
	 * @return The rich curve for this sound pitch
	 */
	const FMovieSceneFloatChannel& GetPitchMultiplierChannel() const { return PitchMultiplier; }

	/**
	 * Return the sound volume
	 *
	 * @param InTime	The position in time within the movie scene
	 * @return The volume the sound will be played with.
	 */
	float GetSoundVolume(FFrameTime InTime) const
	{
		float OutValue = 0.f;
		SoundVolume.Evaluate(InTime, OutValue);
		return OutValue;
	}

	/**
	 * Return the pitch multiplier
	 *
	 * @param Position	The position in time within the movie scene
	 * @return The pitch multiplier the sound will be played with.
	 */
	float GetPitchMultiplier(FFrameTime InTime) const
	{
		float OutValue = 0.f;
		PitchMultiplier.Evaluate(InTime, OutValue);
		return OutValue;
	}

	/**
	 * @return Whether to allow looping if the section length is greater than the sound duration
	 */
	bool GetLooping() const
	{
		return bLooping;
	}

	/**
	 * @return Whether subtitles should be suppressed
	 */
	bool GetSuppressSubtitles() const
	{
		return bSuppressSubtitles;
	}

	/**
	 * @return Whether override settings on this section should be used
	 */
	bool GetOverrideAttenuation() const
	{
		return bOverrideAttenuation;
	}

	/**
	 * @return The attenuation settings
	 */
	UAtomAttenuation* GetAttenuationSettings() const
	{
		return AttenuationSettings;
	}

	/**
	 * @return The attenuation settings
	 */
	const FAtomAttenuationSettings& GetAttenuationOverrides() const
	{
		return AttenuationOverrides;
	}

	/**
	 * @return The attach actor data
	 */
	const FMovieSceneActorReferenceData& GetAttachActorData() const { return AttachActorData; }

	/*
	 * @return The attach component given the bound actor and the actor attach key with the component and socket names
	 */
	USceneComponent* GetAttachComponent(const AActor* InParentActor, const FMovieSceneActorReferenceKey& Key) const;


	/** Called when subtitles are sent to the SubtitleManager.  Set this delegate if you want to hijack the subtitles for other purposes */
	/*
	void SetOnQueueSubtitles(const FOnQueueSubtitles& InOnQueueSubtitles)
	{
		OnQueueSubtitles = InOnQueueSubtitles;
	}
	*/

	/** Called when subtitles are sent to the SubtitleManager.  Set this delegate if you want to hijack the subtitles for other purposes */
	/*
	const FOnQueueSubtitles& GetOnQueueSubtitles() const
	{
		return OnQueueSubtitles;
	}
	*/

	/** called when we finish playing audio, either because it played to completion or because a Stop() call turned it off early */
	void SetOnAudioFinished(const FOnAtomSoundFinished& InOnAtomSoundFinished)
	{
		OnAtomSoundFinished = InOnAtomSoundFinished;
	}

	/** called when we finish playing audio, either because it played to completion or because a Stop() call turned it off early */
	const FOnAtomSoundFinished& GetOnAtomSoundFinished() const
	{
		return OnAtomSoundFinished;
	}

	void SetOnAtomSoundPlaybackPercent(const FOnAtomSoundPlaybackPercent& InOnAtomSoundPlaybackPercent)
	{
		OnAtomSoundPlaybackPercent = InOnAtomSoundPlaybackPercent;
	}

	const FOnAtomSoundPlaybackPercent& GetOnAtomSoundPlaybackPercent() const
	{
		return OnAtomSoundPlaybackPercent;
	}

	/**
	 * @return Whether Playing Sound should be continued When Sequence Is End
	 */
	bool GetContinueSoundWhenSequenceIsEnd() const { return bContinueSoundWhenSequenceIsEnd; }

	TArray<FAtomSelectorParam> GetSelectorLabels() const { return SelectorLabels; }
	int32 GetBlockIndex() const { return FirstBlockIndex; }

	/** Gets the animated scalar parameters and their associated curves. */
	TArray<FScalarParameterNameAndCurve>& GetScalarParameterNamesAndCurves();
	const TArray<FScalarParameterNameAndCurve>& GetScalarParameterNamesAndCurves() const;

	/** Overloads for each input type, const */
	void ForEachInput(TFunction<void(FName, const FMovieSceneBoolChannel&)> InFunction) const { ForEachInternal(InFunction, Inputs_Bool); }
	void ForEachInput(TFunction<void(FName, const FMovieSceneStringChannel&)> InFunction) const { ForEachInternal(InFunction, Inputs_String); }
	void ForEachInput(TFunction<void(FName, const FMovieSceneIntegerChannel&)> InFunction) const { ForEachInternal(InFunction, Inputs_Int); }
	void ForEachInput(TFunction<void(FName, const FMovieSceneFloatChannel&)> InFunction) const { ForEachInternal(InFunction, Inputs_Float); }
	void ForEachInput(TFunction<void(FName, const FMovieSceneAudioTriggerChannel&)> InFunction) const { ForEachInternal(InFunction, Inputs_Trigger); }

public:

	//~ MovieSceneSection interface
	virtual TOptional<TRange<FFrameNumber>> GetAutoSizeRange() const override;
	virtual void TrimSection(FQualifiedFrameTime TrimTime, bool bTrimLeft, bool bDeleteKeys) override;
	virtual UMovieSceneSection* SplitSection(FQualifiedFrameTime SplitTime, bool bDeleteKeys) override;
	virtual TOptional<FFrameTime> GetOffsetTime() const override;
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostEditImport() override;
	virtual EMovieSceneChannelProxyType CacheChannelProxy() override;

	/** Functions setup a currently used AtomComponent while evaluating. */
	FORCEINLINE void SetAtomComponent(UAtomComponent* InAtomComponent) { AtomComponent = InAtomComponent; }

	/** Functions to access currently used AtomComponent while evaluating (may be null). */
	FORCEINLINE UAtomComponent* GetAtomComponent() { return AtomComponent.Get(); }

private:

	template<typename ChannelType, typename ForEachFunction>
	FORCEINLINE static void ForEachInternal(ForEachFunction InFuncton, const TMap<FName, ChannelType>& InMapToIterate)
	{
		for (auto& Item : InMapToIterate)
		{
			InFuncton(Item.Key, Item.Value);
		}
	}


	/** The sound cue or wave that this section plays. */
	UPROPERTY(EditAnywhere, Category = "Atom")
	TObjectPtr<UAtomSoundBase> Sound;

	/** The offset into the beginning of the audio clip. */
	UPROPERTY(EditAnywhere, Category = "Atom")
	FFrameNumber StartFrameOffset;

	/** The volume the sound will be played with. */
	UPROPERTY()
	FMovieSceneFloatChannel SoundVolume;

	/** The pitch multiplier the sound will be played with. */
	UPROPERTY()
	FMovieSceneFloatChannel PitchMultiplier;

	/** Generic inputs for the sound  */
	UPROPERTY()
	TMap<FName, FMovieSceneFloatChannel> Inputs_Float;
	UPROPERTY()
	TMap<FName, FMovieSceneStringChannel> Inputs_String;
	UPROPERTY()
	TMap<FName, FMovieSceneBoolChannel> Inputs_Bool;
	UPROPERTY()
	TMap<FName, FMovieSceneIntegerChannel> Inputs_Int;
	UPROPERTY()
	TMap<FName, FMovieSceneAudioTriggerChannel> Inputs_Trigger;

	UPROPERTY()
	FMovieSceneActorReferenceData AttachActorData;

	/** The scalar parameter names and their associated curves. */
	UPROPERTY()
	TArray<FScalarParameterNameAndCurve> ScalarParameterNamesAndCurves;

	/* Allow looping if the section length is greater than the sound duration */
	UPROPERTY(EditAnywhere, Category = "Atom")
	bool bLooping;

	UPROPERTY(EditAnywhere, Category = "Atom")
	bool bSuppressSubtitles;

	/** Should the attenuation settings on this section be used. */
	UPROPERTY(EditAnywhere, Category = "Attenuation")
	bool bOverrideAttenuation;

	/** The attenuation settings to use. */
	UPROPERTY(EditAnywhere, Category = "Attenuation")
	TObjectPtr<UAtomAttenuation> AttenuationSettings;

	/** If bOverrideSettings is true, the attenuation properties to use for sounds generated by this component */
	UPROPERTY(EditAnywhere, Category = Attenuation, meta = (EditCondition = "bOverrideAttenuation", DisplayAfter = "bOverrideAttenuation", EditConditionHides))
	FAtomAttenuationSettings AttenuationOverrides;

	/** If playback to continue after sequencer ends. */
	UPROPERTY(EditAnywhere, Category = "Atom")
	bool bContinueSoundWhenSequenceIsEnd;

	/** Selector label to set when playing sound (Sound Cue Only) */
	UPROPERTY(EditAnywhere, Category = "Atom")
	TArray<FAtomSelectorParam> SelectorLabels;

	/** Block index to set when playing sound (Sound Cue Only) */
	UPROPERTY(EditAnywhere, Category = "Atom")
	int32 FirstBlockIndex;

	/** Helper to access AtomComponemt used by this section. */
	TWeakObjectPtr<UAtomComponent> AtomComponent;

	/** Called when subtitles are sent to the SubtitleManager.  Set this delegate if you want to hijack the subtitles for other purposes */
	//UPROPERTY()
	//	FOnQueueSubtitles OnQueueSubtitles;

	/** called when we finish playing audio, either because it played to completion or because a Stop() call turned it off early */
	UPROPERTY()
	FOnAtomSoundFinished OnAtomSoundFinished;

	UPROPERTY()
	FOnAtomSoundPlaybackPercent OnAtomSoundPlaybackPercent;

};
