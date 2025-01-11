#include "Character/AnimNotify/SaveInputNotifyState.h"

#include "AnimationUtils.h"

void USaveInputNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                        float TotalDuration)
{
	if (!MeshComp || !MeshComp->GetOwner()) return;
	
	if (const TObjectPtr<APlayerCharacter> Player = Cast<APlayerCharacter>(MeshComp->GetOwner()))
	{
		if (const TObjectPtr<UAbilitySystemComponent> OwnerPlayerAbilitySystem = Player->GetAbilitySystemComponent())
		{
			OwnerPlayerAbilitySystem->AddLooseGameplayTag(Player->GetSaveInputStateTag());
		}
	}
}

void USaveInputNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp && Animation)
	{
		if (const TObjectPtr<APlayerCharacter> Player = Cast<APlayerCharacter>(MeshComp->GetOwner()))
		{
			if (const TObjectPtr<UAbilitySystemComponent> OwnerPlayerAbilitySystem = Player->GetAbilitySystemComponent())
			{
				OwnerPlayerAbilitySystem->RemoveLooseGameplayTag(Player->GetSaveInputStateTag());
			}
		}
	}
}
