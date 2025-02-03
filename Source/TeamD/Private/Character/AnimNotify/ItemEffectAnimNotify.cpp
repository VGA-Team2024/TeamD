#include "Character/AnimNotify/ItemEffectAnimNotify.h"
#include "Character/CharacterBase.h"
#include "Framework/CustomFramework.h"
#include "GAS/CustomAbilitySystemComponent.h"
#include "GAS/Player/ItemAbilityBase.h"

void UItemEffectAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	// 再生中のItemAbilityのアイテム効果発動のイベントを発行する
	if (const TObjectPtr<AActor> OwnerActor = MeshComp->GetOwner())
	{
		if (const TObjectPtr<ACharacterBase> OwnerCharacter = Cast<ACharacterBase>(OwnerActor))
		{
			if (const TObjectPtr<UCustomAbilitySystemComponent> OwnerCustomASC = OwnerCharacter->GetAbilitySystemComponent())
			{
				TArray<UGameplayAbility*> ActiveAbilities = OwnerCustomASC->GetActiveAbilities();

				if (ActiveAbilities.Num() > 0)
				{
					if (TObjectPtr<UItemAbilityBase> UsingItemAbility = Cast<UItemAbilityBase>(ActiveAbilities[0]))
					{
						UsingItemAbility->OnActivateEffect.Broadcast();
						return;
					}
					
					LOG_INFO(Warning, "cast failed item ability")
				}
				else LOG_INFO(Warning, "active num is 0");
			}
		}
	}
	
	LOG_INFO(Warning, "nullptr for any");
}
