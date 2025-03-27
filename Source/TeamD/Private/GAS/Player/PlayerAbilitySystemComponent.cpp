#include "GAS/Player/PlayerAbilitySystemComponent.h"

void UPlayerAbilitySystemComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TryActivateAbilitiesBySavedInputTagInTick(DeltaTime);
}

void UPlayerAbilitySystemComponent::SaveTagTryActivateAbilities(const FGameplayTag InputTag)
{
	if (!InputTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid InputTag:%d"), __LINE__);
		return;
	}
	
	// Activate出来なかった場合
	if (!TryActivateAbilitiesByTag(FGameplayTagContainer(InputTag), true))
	{
		SavedInputTag = InputTag;
		// 入力Tagを上書き
		RemoveInputTags();
		// タイマーリセット
		SaveInputTimer = TimeToSaveInput;
		AddLooseGameplayTag(InputTag);
	}
}

void UPlayerAbilitySystemComponent::RemoveInputTags()
{
	RemoveTagsWithParent(InputTagRoot);
	SaveInputTimer = 0;
}

void UPlayerAbilitySystemComponent::TryActivateAbilitiesBySavedInputTagInTick(float DeltaTime)
{
	if (SaveInputTimer <= 0 ||
		(SavedInputTag.IsValid() && TryActivateAbilitiesByTag(FGameplayTagContainer(SavedInputTag), true)))
	{
		// タイマーリセット
		SaveInputTimer = 0;
		// Tagをリセット
		SavedInputTag = FGameplayTag::EmptyTag;
		// InputTagを削除
		RemoveTagsWithParent(InputTagRoot);
		
		return;
	}

	// タイマーを進める
	SaveInputTimer -= DeltaTime;
}