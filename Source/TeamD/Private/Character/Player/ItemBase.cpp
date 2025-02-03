#include "Character/Player/ItemBase.h"
#include "Framework/CustomFramework.h"

void UItemBase::Init(const TObjectPtr<UCustomAbilitySystemComponent>& CustomAbilitySystemComponent)
{
	OwnerCustomASC = CustomAbilitySystemComponent;
}

void UItemBase::UseItem_Implementation()
{
	const FGameplayAbilitySpec Spec = OwnerCustomASC->GiveAbilityAndActivateOnce(ItemAbilityClass);
	// Abilityを再生 再生できなかったら終了
	if (!Spec.IsActive())
	{
		LOG_INFO(Log, "item ability not active");
		return;
	}
	
	TObjectPtr<UItemAbilityBase> ActiveItemAbility = Cast<UItemAbilityBase>(OwnerCustomASC->GetActiveAbility(Spec.Handle));
	if (!ActiveItemAbility)
	{
		LOG_INFO(Error, "failed cast to item ability");
		return;
	}

	// アイテム効果のイベントに登録
	ActiveItemAbility->OnActivateEffect.AddDynamic(this, &UItemBase::ApplyItemEffect);
}

void UItemBase::ApplyItemEffect_Implementation()
{
	CurrentStackNum--;
	
	LOG_INFO(Log, TEXT("%d"), CurrentStackNum);
}

void UItemBase::AddStack(int32 Value)
{
	CurrentStackNum = FMath::Min(CurrentStackNum + Value, MaxStackNum);
}

int32 UItemBase::GetMaxStack() const { return MaxStackNum; }

int32 UItemBase::GetCurrentStack() const { return CurrentStackNum; }
