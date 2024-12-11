/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : CriWareMovieScene
 * File     : MovieSceneAtomTrack.h
 *
 ****************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "MovieSceneNameableTrack.h"
#include "Compilation/IMovieSceneTrackTemplateProducer.h"
#include "Evaluation/MovieSceneEvalTemplate.h"

#include "MovieSceneAtomTrack.generated.h"

// Forward Definitions
class UAtomSoundBase;

namespace AtomTrackConstants
{
	/* Preview time when moving the cursor in the sequencer editor
	 * Note: Considering the buffering time of stream playback,
     * Set the preview time longer than the UE standard sound. */
	const float ScrubDuration = 0.050f;
}

/**
 * Handles manipulation of audio.
 */
UCLASS(MinimalAPI)
class UMovieSceneAtomTrack
	: public UMovieSceneNameableTrack
	, public IMovieSceneTrackTemplateProducer
{
	GENERATED_BODY()

public:

	UMovieSceneAtomTrack(const FObjectInitializer& ObjectInitializer);

	/** Adds a new sound cue to the audio */
	virtual UMovieSceneSection* AddNewSoundOnRow(UAtomSoundBase* Sound, FFrameNumber Time, int32 RowIndex);

	/** Adds a new sound cue on the next available/non-overlapping row */
	virtual UMovieSceneSection* AddNewSound(UAtomSoundBase* Sound, FFrameNumber Time) { return AddNewSoundOnRow(Sound, Time, INDEX_NONE); }

	/** @return The audio sections on this track */
	const TArray<UMovieSceneSection*>& GetAtomSections() const
	{
		return AtomSections;
	}

public:
	//~ UMovieSceneTrack Interface
	virtual bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override;
	virtual void RemoveAllAnimationData() override;
	virtual bool HasSection(const UMovieSceneSection& Section) const override;
	virtual void AddSection(UMovieSceneSection& Section) override;
	virtual void RemoveSection(UMovieSceneSection& Section) override;
	virtual void RemoveSectionAt(int32 SectionIndex) override;
	virtual bool IsEmpty() const override;
	virtual const TArray<UMovieSceneSection*>& GetAllSections() const override;
	virtual bool SupportsMultipleRows() const override;
	virtual UMovieSceneSection* CreateNewSection() override;
	virtual FMovieSceneTrackRowSegmentBlenderPtr GetRowSegmentBlender() const override;

	// ~IMovieSceneTrackTemplateProducer interface
	virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override;

private:

	/** List of all root Atom sections */
	UPROPERTY()
	TArray<TObjectPtr<UMovieSceneSection>> AtomSections;

#if WITH_EDITORONLY_DATA

public:

	/**
	 * Get the height of this track's rows
	 */
	int32 GetRowHeight() const
	{
		return RowHeight;
	}

	/**
	 * Set the height of this track's rows
	 */
	void SetRowHeight(int32 NewRowHeight)
	{
		RowHeight = FMath::Max(16, NewRowHeight);
	}

private:

	/** The height for each row of this track */
	UPROPERTY()
	int32 RowHeight;

#endif
};
