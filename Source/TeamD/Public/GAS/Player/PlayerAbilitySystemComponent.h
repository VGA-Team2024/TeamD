#pragma once

#include "CoreMinimal.h"
#include "GAS/CustomAbilitySystemComponent.h"
#include "PlayerAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class TEAMD_API UPlayerAbilitySystemComponent : public UCustomAbilitySystemComponent
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	//------------------------入力保存------------------------

protected:
	// 入力を保存する時間
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = SaveInput)
	float TimeToSaveInput;

	// 入力保存のTagのRoot
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = SaveInput)
	FGameplayTag InputTagRoot;

	FGameplayTag SavedInputTag;

	// 入力保存のタイマー
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = SaveInput)
	float SaveInputTimer = 0.f;

public:
	/**
	 * タグを元にAbilityを再生する
	 * 再生できなければ、一定時間再生できるまでTryActivateする
	 * @param InputTag 入力Tag
	 */
	void SaveTagTryActivateAbilities(const FGameplayTag InputTag);

	// 入力保存中であるか
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SaveInput")
	bool IsSavingInput() const { return SaveInputTimer > 0.f; }

	// 入力のTagを削除
	UFUNCTION(BlueprintCallable, Category = "SaveInput")
	void RemoveInputTags();
	
protected:
	// 一定時間SavedInputTagでTryActivateする
	void TryActivateAbilitiesBySavedInputTagInTick(float DeltaTime);
};
