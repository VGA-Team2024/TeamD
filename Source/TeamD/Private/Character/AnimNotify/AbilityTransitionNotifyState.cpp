#include "Character/AnimNotify/AbilityTransitionNotifyState.h"

void UAbilityTransitionNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration)
{
	OwnerPlayer = Cast<APlayerCharacter>(MeshComp->GetOwner());

	if (OwnerPlayer)
	{
		PlayerAbilitySystemComponent = OwnerPlayer->GetAbilitySystemComponent();
	}
}

void UAbilityTransitionNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	// 保存したInputのTagがなければreturn
	if (!OwnerPlayer || !PlayerAbilitySystemComponent->HasMatchingGameplayTag(SaveInputTagRoot)) return;

	// Abilityを起動してみる
	for (const auto Ability : TransitionalAbilities)
	{
		OwnerPlayer->GetAbilitySystemComponent()->TryActivateAbilityByClass(Ability);
	}

	// 持ってるTagをループする
	for (FGameplayTag Tag : PlayerAbilitySystemComponent->GetOwnedGameplayTags().GetGameplayTagArray())
	{
		// SaveInputだったら消す
		if (Tag.MatchesTag(SaveInputTagRoot) || Tag == SaveInputTagRoot)
		{
			PlayerAbilitySystemComponent->RemoveLooseGameplayTag(Tag);
		}
	}
}
