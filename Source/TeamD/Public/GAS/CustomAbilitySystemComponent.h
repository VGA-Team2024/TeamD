#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CustomAbilitySystemComponent.generated.h"

/**
 * AbilitySystemComponentを拡張するクラス
 */
UCLASS()
class TEAMD_API UCustomAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// Abilityを発動して捨てる
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	FGameplayAbilitySpecHandle GiveAbilityAndActivateOnce(TSubclassOf<UGameplayAbility> AbilityClass);

	// 特定のAbilityを削除する
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void RemoveAbilityByClass(TSubclassOf<UGameplayAbility> AbilityClass);

	UFUNCTION()
	void OnAbilityEnded(UGameplayAbility* Ability);
};
