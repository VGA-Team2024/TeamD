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
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//------------------------Ability------------------------
	
	/**
	 * Abilityを発動して捨てる
	 * @param AbilityClass 再生するAbilityClass
	 * @return 再生したAbilitySpec
	 */
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	FGameplayAbilitySpec GiveAbilityAndActivateOnce(TSubclassOf<UGameplayAbility> AbilityClass);
	
	// Ability参照からAbilityを削除する
	UFUNCTION(blueprintcallable, Category = "Abilities")
	void RemoveAbilityByReference(UGameplayAbility* Ability);

	// 現在ActiveなAbilityを取得する
	UFUNCTION(BlueprintCallable, BlueprintPure, category = "Abilities")
	TArray<UGameplayAbility*> GetActiveAbilities();

	/**
	 * 指定したSpecHandleで最初に取得した再生中のAbility参照を返す
	 * @param SpecHandle 取得するAbilityのSpecHandle
	 * @return 再生中のAbility参照　再生中でなければnullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	UGameplayAbility* GetActiveAbility(FGameplayAbilitySpecHandle SpecHandle);

	//------------------------Tag------------------------

	// 特定のTag以下のTagを全て削除する
	UFUNCTION(BlueprintCallable, Category = "Tag")
	void RemoveTagsWithParent(const FGameplayTag& ParentTag);
};
