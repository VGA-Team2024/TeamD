/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : CriWareMovieScene
 * File     : MovieSceneAtomTrack.cpp
 *
 ****************************************************************************/

#include "MovieSceneAtomTrack.h"

#include "MovieScene.h"
#include "Evaluation/MovieSceneSegment.h"
#include "Compilation/MovieSceneSegmentCompiler.h"
#include "Compilation/MovieSceneCompilerRules.h"
#include "MovieSceneCommonHelpers.h"

#include "Atom/AtomSoundBase.h"

#include "MovieSceneAtomSection.h"
#include "MovieSceneAtomTemplate.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MovieSceneAtomTrack)

#define LOCTEXT_NAMESPACE "MovieSceneAtomTrack"

UMovieSceneAtomTrack::UMovieSceneAtomTrack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedBlendTypes.Add(EMovieSceneBlendType::Absolute);
#if WITH_EDITORONLY_DATA
	TrackTint = FColor(93, 95, 136);
	RowHeight = 50;
#endif
}

FMovieSceneEvalTemplatePtr UMovieSceneAtomTrack::CreateTemplateForSection(const UMovieSceneSection& InSection) const
{
	return FMovieSceneAtomSectionTemplate(*CastChecked<UMovieSceneAtomSection>(&InSection));
}

const TArray<UMovieSceneSection*>& UMovieSceneAtomTrack::GetAllSections() const
{
	return AtomSections;
}

bool UMovieSceneAtomTrack::SupportsMultipleRows() const
{
	return true;
}

bool UMovieSceneAtomTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UMovieSceneAtomSection::StaticClass();
}

void UMovieSceneAtomTrack::RemoveAllAnimationData()
{
	AtomSections.Empty();
}

bool UMovieSceneAtomTrack::HasSection(const UMovieSceneSection& Section) const
{
	return AtomSections.Contains(&Section);
}

void UMovieSceneAtomTrack::AddSection(UMovieSceneSection& Section)
{
	AtomSections.Add(&Section);
}

void UMovieSceneAtomTrack::RemoveSection(UMovieSceneSection& Section)
{
	AtomSections.Remove(&Section);
}

void UMovieSceneAtomTrack::RemoveSectionAt(int32 SectionIndex)
{
	AtomSections.RemoveAt(SectionIndex);
}

bool UMovieSceneAtomTrack::IsEmpty() const
{
	return AtomSections.Num() == 0;
}

UMovieSceneSection* UMovieSceneAtomTrack::AddNewSoundOnRow(UAtomSoundBase* Sound, FFrameNumber Time, int32 RowIndex)
{
	check(Sound);

	// get frame rate
	FFrameRate FrameRate = GetTypedOuter<UMovieScene>()->GetTickResolution();

	// get length of audio data in seconds
	float SoundDuration = Sound->GetDuration(); // seconds
	if (SoundDuration == ATOM_INDEFINITELY_LOOPING_DURATION)
	{
		SoundDuration = Sound->WaveInfo.GetDuration();
	}

	// Convert to frame units
	FFrameTime DurationToUse = SoundDuration * FrameRate;

	// add section
	UMovieSceneAtomSection* NewSection = NewObject<UMovieSceneAtomSection>(this);
	NewSection->InitialPlacementOnRow(AtomSections, Time, DurationToUse.FrameNumber.Value, RowIndex);
	NewSection->SetSound(Sound);

	AtomSections.Add(NewSection);

	return NewSection;
}

UMovieSceneSection* UMovieSceneAtomTrack::CreateNewSection()
{
	return NewObject<UMovieSceneAtomSection>(this, NAME_None, RF_Transactional);
}

FMovieSceneTrackRowSegmentBlenderPtr UMovieSceneAtomTrack::GetRowSegmentBlender() const
{
	struct FBlender : FMovieSceneTrackRowSegmentBlender
	{
		virtual void Blend(FSegmentBlendData& BlendData) const override
		{
			// Run the default high pass filter for overlap priority
			MovieSceneSegmentCompiler::FilterOutUnderlappingSections(BlendData);

			// Weed out based on array index (legacy behaviour)
			MovieSceneSegmentCompiler::BlendSegmentLegacySectionOrder(BlendData);
		}
	};
	return FBlender();
}

#undef LOCTEXT_NAMESPACE
