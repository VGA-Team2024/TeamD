#pragma once

#include "CoreMinimal.h"
#include "Character/Player/ItemBase.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "HealItem.generated.h"

/**
 * 回復アイテム
 */
UCLASS()
class TEAMD_API UHealItem : public UItemBase
{
	GENERATED_BODY()

public:
	// 回復アイテムの効果
	virtual void ApplyItemEffect_Implementation() override;

protected:
	// 回復量
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Effect)
	float AmountHeal;

	// 回復用のAbilityEffectClass
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Effect)
	TSubclassOf<UGameplayEffect> HealEffectClass;

	// AbilityEffectの値変更用のTag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Effect)
	FGameplayTag SetHealMagnitudeTag;
};
