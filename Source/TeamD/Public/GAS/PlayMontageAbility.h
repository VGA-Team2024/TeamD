#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PlayMontageAbility.generated.h"

/**
 * 
 */
UCLASS()
class TEAMD_API UPlayMontageAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// EndAbilityをAddDynamicするため
	UFUNCTION()
	void CallEndAbility();

	// モーションアニメーション
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> MontageToPlay;
};
