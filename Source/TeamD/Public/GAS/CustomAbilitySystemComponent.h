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
	
	// Abilityを発動して捨てる
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	FGameplayAbilitySpecHandle GiveAbilityAndActivateOnce(TSubclassOf<UGameplayAbility> AbilityClass);

	// 特定のAbilityを削除する
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void RemoveAbilityByClass(TSubclassOf<UGameplayAbility> AbilityClass);

	// GiveAbilityAndActivateOnceでAbility終了を受け取る用
	UFUNCTION()
	void OnAbilityEnded(UGameplayAbility* Ability);

	//------------------------入力保存------------------------

	/**
	 * タグを元にAbilityを再生する
	 * 再生できなければ、一定時間再生できるまでTryActivateする
	 * @param InputTag 入力Tag
	 */
	void SaveTagTryActivateAbilities(const FGameplayTag InputTag);

	// 一定時間SavedInputTagでTryActivateする
	void TryActivateAbilitiesBySavedInputTagInTick(float DeltaTime);

	FGameplayTag SavedInputTag;

	// 入力を保存する時間
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = SaveInput)
	float TimeToSaveInput;

	// 入力保存のタイマー
	float SaveInputTimer = 0.f;
};
