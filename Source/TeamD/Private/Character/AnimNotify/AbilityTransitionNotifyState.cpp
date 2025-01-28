#include "Character/AnimNotify/AbilityTransitionNotifyState.h"

void UAbilityTransitionNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	const TObjectPtr<APlayerCharacter> OwnerPlayer = Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (!OwnerPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("Owner is not player:%d%d"), __FILE__, __LINE__);
		return;
	}
	const TObjectPtr<UCustomAbilitySystemComponent> PlayerAbilitySystemComponent = OwnerPlayer->GetAbilitySystemComponent();
	if (!PlayerAbilitySystemComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("custom asc is nullptr:%d%d"), __FILE__, __LINE__);
		return;
	}

	// 入力保存してないなら終了
	if (!PlayerAbilitySystemComponent->IsSavingInput()) return;
	
	// Abilityを起動してみる
	for (const auto Ability : TransitionalAbilities)
	{
		if (PlayerAbilitySystemComponent->TryActivateAbilityByClass(Ability))
		{
			// 入力Tagを削除して終了
			PlayerAbilitySystemComponent->RemoveInputTags();
			return;
		}
	}
	
	// const FGameplayTag SaveInputTagRoot = OwnerPlayer->GetSaveInputStateTagRoot();
	//
	// // 保存したInputのTagがなければreturn
	// if (!PlayerAbilitySystemComponent || !PlayerAbilitySystemComponent->HasMatchingGameplayTag(SaveInputTagRoot)) return;
	//
	// // Abilityを起動してみる
	// for (const auto Ability : TransitionalAbilities)
	// {
	// 	OwnerPlayer->GetAbilitySystemComponent()->TryActivateAbilityByClass(Ability);
	// }
	//
	// // ループ中に削除するとバグるので削除予定のTagを一時保存しておく
	// TArray<FGameplayTag> TagsToRemove;
	//
	// while (PlayerAbilitySystemComponent->HasMatchingGameplayTag(SaveInputTagRoot))
	// {
	// 	// 持ってるTagをループする
	// 	for (FGameplayTag Tag : PlayerAbilitySystemComponent->GetOwnedGameplayTags().GetGameplayTagArray())
	// 	{
	// 		// SaveInputだったら消す
	// 		if (Tag.MatchesTag(SaveInputTagRoot) || Tag == SaveInputTagRoot)
	// 		{
	// 			TagsToRemove.Add(Tag);
	// 		}
	// 	}
	//
	// 	// 後で削除
	// 	for (const FGameplayTag Tag : TagsToRemove)
	// 	{
	// 		PlayerAbilitySystemComponent->RemoveLooseGameplayTag(Tag);
	// 	}
	// }
}
