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

	//------------------------入力保存------------------------

	// 入力保存を使用するかどうか todo:SaveInputはASCの機能ではないから別に移したい
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = SaveInput)
	bool bUseSaveInput = false;

	/**
	 * タグを元にAbilityを再生する
	 * 再生できなければ、一定時間再生できるまでTryActivateする
	 * @param InputTag 入力Tag
	 */
	void SaveTagTryActivateAbilities(const FGameplayTag InputTag);

	// 一定時間SavedInputTagでTryActivateする
	void TryActivateAbilitiesBySavedInputTagInTick(float DeltaTime);

	// 入力のTagを削除
	UFUNCTION(BlueprintCallable, Category = "SaveInput")
	void RemoveInputTags();

	FGameplayTag SavedInputTag;

	// 入力保存のTagのRoot
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = SaveInput)
	FGameplayTag InputTagRoot;

	// 入力を保存する時間
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = SaveInput)
	float TimeToSaveInput;

	// 入力保存のタイマー
	float SaveInputTimer = 0.f;

	// 入力保存中であるか
	UFUNCTION(BlueprintPure, Category = "SaveInput")
	bool IsSavingInput() const { return SaveInputTimer > 0.f; }

	//------------------------Tag------------------------

	// 特定のTag以下のTagを全て削除する
	UFUNCTION(BlueprintCallable, Category = "Tag")
	void RemoveTagsWithParent(const FGameplayTag& ParentTag);
};
