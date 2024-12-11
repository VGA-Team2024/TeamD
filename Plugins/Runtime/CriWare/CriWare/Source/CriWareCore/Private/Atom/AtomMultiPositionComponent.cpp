/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCore
 * File     : AtomMultiPositionComponent.cpp
 *
 ****************************************************************************/

#include "Atom/AtomMultiPositionComponent.h"

UAtomMultiPositionComponent::UAtomMultiPositionComponent(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAtomMultiPositionComponent::GetAllPositions(TArray<FTransform>& Positions) const
{
	Positions.Reset();

	if (MultiPositionSettings.bUseComponentPositionAsSource)
	{
		Positions.Emplace(GetComponentTransform());
	}

	for (auto Actor : MultiPositionSettings.SourceActors)
	{
		if (Actor)
		{
			if (MultiPositionSettings.SourceComponentClass)
			{
				TInlineComponentArray<UActorComponent*> Components;
				Actor->GetComponents(MultiPositionSettings.SourceComponentClass, Components);
				for (auto& Component : Components)
				{
					if (USceneComponent* SceneComponent = Cast<USceneComponent>(Component))
					{
						Positions.Emplace(SceneComponent->GetComponentTransform());
					}
				}
			}
			else
			{
				Positions.Emplace(Actor->GetActorTransform());
			}
		}
	}

	Positions.Append(MultiPositionSettings.SourcePositions);
}

void UAtomMultiPositionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdatePositions();
}

void UAtomMultiPositionComponent::UpdatePositions()
{
	if (IsPlaying())
	{
		if (MultiPositionSettings.SourceActors.IsEmpty() && !MultiPositionSettings.bUseComponentPositionAsSource)
		{
			SetMultiplePositions(MultiPositionSettings.SourcePositions, EAtomMultiPositionType::MultiSources);
		}
		else
		{
			TArray<FTransform> Positions;
			GetAllPositions(Positions);
			SetMultiplePositions(Positions, EAtomMultiPositionType::MultiSources);
		}
	}
}

UAtomComponentExtension* UAtomMultiPositionComponent::GetComponentExtension()
{
	UAtomMultiPositionComponentExtension* Extension = NewObject<UAtomMultiPositionComponentExtension>(this, NAME_None, RF_Transactional | RF_Transient | RF_TextExportTransient);
	if (Extension)
	{
		Extension->AtomMultiPositionComponent = this;
	}

	return Extension;
}
