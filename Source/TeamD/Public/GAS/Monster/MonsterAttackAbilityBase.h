#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MonsterAttackAbilityBase.generated.h"

/**
 * モンスターの攻撃アビリティ基底クラス
 */
UCLASS()
class TEAMD_API UMonsterAttackAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	// EndAbilityをAddDynamicするため
	UFUNCTION()
	void CallEndAbility();
	
	// モーション値
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Parameter)
	int MotionValue;

	// モーションアニメーション
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Parameter)
	TObjectPtr<UAnimMontage> MontageToPlay;
};
