#include "Character/AnimNotify/AbilityTransitionNotifyState.h"

void UAbilityTransitionNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration)
{
	OwnerPlayer = Cast<APlayerCharacter>(MeshComp->GetOwner());
}

void UAbilityTransitionNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	// InputのTagがなければreturn
	if (!OwnerPlayer || OwnerPlayer->SaveInputTag == FGameplayTag::EmptyTag) return;

	for (const auto Ability : TransitionalAbilities)
	{
		OwnerPlayer->GetAbilitySystemComponent()->TryActivateAbilityByClass(Ability);
	}

	// 入力は消費したら消す
	OwnerPlayer->SaveInputTag = FGameplayTag::EmptyTag;
}
