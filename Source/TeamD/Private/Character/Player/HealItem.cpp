#include "Character/Player/HealItem.h"
#include "GameplayEffectTypes.h"
#include "Framework/CustomFramework.h"

void UHealItem::ApplyItemEffect_Implementation()
{
	Super::ApplyItemEffect_Implementation();

	// 自身を回復するアイテム効果
	if (OwnerCustomASC && HealEffectClass)
	{
		// Spec作成
		const FGameplayEffectSpecHandle SpecHandle =
			OwnerCustomASC->MakeOutgoingSpec(HealEffectClass, 0, OwnerCustomASC->MakeEffectContext());

		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(SetHealMagnitudeTag, AmountHeal);
			// Effectの適用
			OwnerCustomASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), OwnerCustomASC);
		}
	}
	else LOG_INFO(Error, "nullptr")
}
