/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : CriWareMovieScene
 * File     : MovieSceneAtomSection.cpp
 *
 ****************************************************************************/

#include "MovieSceneAtomSection.h"

#include "UObject/SequencerObjectVersion.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "GameFramework/Actor.h"
#include "MovieScene.h"
#include "MovieSceneCommonHelpers.h"
#include "Misc/FrameRate.h"
#include "Misc/GeneratedTypeName.h"

#include "Atom/AtomSoundBase.h"
#include "Atom/AtomSoundCue.h"

#include "MovieSceneAtomTemplate.h"
#include "MovieSceneAtomTrack.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MovieSceneAtomSection)

#if WITH_EDITOR
struct FAtomChannelEditorData
{
	FAtomChannelEditorData()
	{
		Data[0].SetIdentifiers("Volume", NSLOCTEXT("MovieSceneAtomSection", "SoundVolumeText", "Volume"));
		Data[1].SetIdentifiers("Pitch", NSLOCTEXT("MovieSceneAtomSection", "PitchText", "Pitch"));
		Data[2].SetIdentifiers("AttachActor", NSLOCTEXT("MovieSceneAtomSection", "AttachActorText", "Attach"));
	}

	FMovieSceneChannelMetaData Data[3];
};
#endif // WITH_EDITOR

namespace
{
	FFrameNumber GetStartOffsetAtTrimTime(FQualifiedFrameTime TrimTime, FFrameNumber StartOffset, FFrameNumber StartFrame)
	{
		return StartOffset + TrimTime.Time.FrameNumber - StartFrame;
	}
}

UMovieSceneAtomSection::UMovieSceneAtomSection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Sound = nullptr;
	bLooping = true;
	bSuppressSubtitles = false;
	bOverrideAttenuation = false;
	BlendType = EMovieSceneBlendType::Absolute;

	EvalOptions.EnableAndSetCompletionMode(GetLinkerCustomVersion(FSequencerObjectVersion::GUID) < FSequencerObjectVersion::WhenFinishedDefaultsToProjectDefault ?
		EMovieSceneCompletionMode::RestoreState :
		EMovieSceneCompletionMode::ProjectDefault);

	SoundVolume.SetDefault(1.f);
	PitchMultiplier.SetDefault(1.f);
	ScalarParameterNamesAndCurves.Empty();
	
	FirstBlockIndex = 0;

	bContinueSoundWhenSequenceIsEnd = false;
}

void UMovieSceneAtomSection::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsLoading())
	{
		CacheChannelProxy();
	}
}

void UMovieSceneAtomSection::PostEditImport()
{
	Super::PostEditImport();

	CacheChannelProxy();
}

TArray<FScalarParameterNameAndCurve>& UMovieSceneAtomSection::GetScalarParameterNamesAndCurves()
{
	return ScalarParameterNamesAndCurves;
}

const TArray<FScalarParameterNameAndCurve>& UMovieSceneAtomSection::GetScalarParameterNamesAndCurves() const
{
	return ScalarParameterNamesAndCurves;
}

EMovieSceneChannelProxyType  UMovieSceneAtomSection::CacheChannelProxy()
{
	// Set up the channel proxy
	FMovieSceneChannelProxyData Channels;

	UMovieSceneAtomTrack* AtomTrack = Cast<UMovieSceneAtomTrack>(GetOuter());
	UMovieScene* MovieScene = AtomTrack ? Cast<UMovieScene>(AtomTrack->GetOuter()) : nullptr;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
	const bool bHasAttachData = MovieScene && MovieScene->ContainsTrack(*AtomTrack);
#else
	const bool bHasAttachData = AtomTrack && MovieScene->IsAMasterTrack(*AtomTrack);
#endif

#if WITH_EDITOR

	FAtomChannelEditorData EditorData;
	
	int32 SortOrder = 0;
	EditorData.Data[0].SortOrder = SortOrder++;
	EditorData.Data[1].SortOrder = SortOrder++;
	EditorData.Data[2].SortOrder = SortOrder++;
	
	Channels.Add(SoundVolume, EditorData.Data[0], TMovieSceneExternalValue<float>());
	Channels.Add(PitchMultiplier, EditorData.Data[1], TMovieSceneExternalValue<float>());
	if (bHasAttachData)
	{
		Channels.Add(AttachActorData, EditorData.Data[2]);
	}

#else

	Channels.Add(SoundVolume);
	Channels.Add(PitchMultiplier);
	if (bHasAttachData)
	{
		Channels.Add(AttachActorData);
	}

#endif

	if (auto SoundCue = Cast<UAtomSoundCue>(Sound))
	{
		TArray<FString> AisacControls;

		AisacControls = SoundCue->CueInfo.AisacControlNames;

		for (FString AisacName : AisacControls)
		{
			ScalarParameterNamesAndCurves.Add(FScalarParameterNameAndCurve(*AisacName));
			FScalarParameterNameAndCurve& AisacControl = ScalarParameterNamesAndCurves.Last();

#if WITH_EDITOR
			FMovieSceneChannelMetaData MetaData(AisacControl.ParameterName, FText::FromName(AisacControl.ParameterName));
			MetaData.SortOrder = SortOrder++;
			AisacControl.ParameterCurve.SetDefault(0.0f);
			Channels.Add(AisacControl.ParameterCurve, MetaData, TMovieSceneExternalValue<float>());
#else
			AisacControl.ParameterCurve.SetDefault(0.0f);
			Channels.Add(AisacControl.ParameterCurve);
#endif
		}
	}

	ChannelProxy = MakeShared<FMovieSceneChannelProxy>(MoveTemp(Channels));

	return EMovieSceneChannelProxyType::Dynamic;
}

TOptional<FFrameTime> UMovieSceneAtomSection::GetOffsetTime() const
{
	return TOptional<FFrameTime>(StartFrameOffset);
}

TOptional<TRange<FFrameNumber>> UMovieSceneAtomSection::GetAutoSizeRange() const
{
	if (!Sound) {
		return TRange<FFrameNumber>();
	}

	// get frame rate
	FFrameRate FrameRate = GetTypedOuter<UMovieScene>()->GetTickResolution();

	// get length of audio data in seconds
	float SoundDuration = Sound->GetDuration();
	if (SoundDuration == ATOM_INDEFINITELY_LOOPING_DURATION)
	{
		SoundDuration = Sound->WaveInfo.GetDuration();
	}

	// Convert to frame units
	FFrameTime DurationToUse = SoundDuration * FrameRate;

	return TRange<FFrameNumber>(GetInclusiveStartFrame(), GetInclusiveStartFrame() + DurationToUse.FrameNumber);
}

void UMovieSceneAtomSection::TrimSection(FQualifiedFrameTime TrimTime, bool bTrimLeft, bool bDeleteKeys)
{
	SetFlags(RF_Transactional);

	if (TryModify())
	{
		if (bTrimLeft)
		{
			StartFrameOffset = HasStartFrame() ? GetStartOffsetAtTrimTime(TrimTime, StartFrameOffset, GetInclusiveStartFrame()) : 0;
		}

		Super::TrimSection(TrimTime, bTrimLeft, bDeleteKeys);
	}
}

UMovieSceneSection* UMovieSceneAtomSection::SplitSection(FQualifiedFrameTime SplitTime, bool bDeleteKeys)
{
	const FFrameNumber InitialStartFrameOffset = StartFrameOffset;

	const FFrameNumber NewOffset = HasStartFrame() ? GetStartOffsetAtTrimTime(SplitTime, StartFrameOffset, GetInclusiveStartFrame()) : 0;

	UMovieSceneSection* NewSection = Super::SplitSection(SplitTime, bDeleteKeys);
	if (NewSection != nullptr)
	{
		UMovieSceneAtomSection* NewAtomSection = Cast<UMovieSceneAtomSection>(NewSection);
		NewAtomSection->StartFrameOffset = NewOffset;
	}

	// Restore original offset modified by splitting
	StartFrameOffset = InitialStartFrameOffset;

	return NewSection;
}

void UMovieSceneAtomSection::SetSound(UAtomSoundBase* InSound)
{
	Sound = InSound;
	CacheChannelProxy();
}

USceneComponent* UMovieSceneAtomSection::GetAttachComponent(const AActor* InParentActor, const FMovieSceneActorReferenceKey& Key) const
{
	// Get the component to which AtomCompoent should be attached
	FName AttachComponentName = Key.ComponentName;
	FName AttachSocketName = Key.SocketName;

	if (AttachSocketName != NAME_None)
	{
		// If Socket is specified, attach to Socket position
		if (AttachComponentName != NAME_None)
		{
			// If the specified component name is found and the socket exists, return that component
			TInlineComponentArray<USceneComponent*> PotentialAttachComponents(InParentActor);
			for (USceneComponent* PotentialAttachComponent : PotentialAttachComponents)
			{
				if (PotentialAttachComponent->GetFName() == AttachComponentName && PotentialAttachComponent->DoesSocketExist(AttachSocketName))
				{
					return PotentialAttachComponent;
				}
			}
		}
		else if (InParentActor->GetRootComponent()->DoesSocketExist(AttachSocketName))
		{
			return InParentActor->GetRootComponent();
		}
	}
	else if (AttachComponentName != NAME_None)
	{
		// Returns the component with the specified name if the Socket name is not specified
		TInlineComponentArray<USceneComponent*> PotentialAttachComponents(InParentActor);
		for (USceneComponent* PotentialAttachComponent : PotentialAttachComponents)
		{
			if (PotentialAttachComponent->GetFName() == AttachComponentName)
			{
				return PotentialAttachComponent;
			}
		}
	}

	if (InParentActor->GetDefaultAttachComponent())
	{
		// If the actor has a default attached component, return that component.
		return InParentActor->GetDefaultAttachComponent();
	}
	else
	{
		return InParentActor->GetRootComponent();
	}
}
