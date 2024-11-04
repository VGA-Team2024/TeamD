#include "Character/AnimNotify/SaveInputNotifyState.h"

void USaveInputNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration)
{
	if (!MeshComp || !MeshComp->GetOwner()) return;
	
	if (const TObjectPtr<APlayerCharacter> Player = Cast<APlayerCharacter>(MeshComp->GetOwner()))
	{
		if ((OwnerPlayerAbilitySystem = Player->GetAbilitySystemComponent()))
		{
			OwnerPlayerAbilitySystem->AddLooseGameplayTag(SaveInputStateTag);
		}
	}
}

void USaveInputNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (OwnerPlayerAbilitySystem != nullptr || !SaveInputStateTag.IsValid())
	{
		OwnerPlayerAbilitySystem->RemoveLooseGameplayTag(SaveInputStateTag);
	}
}
