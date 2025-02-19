#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "LevelSequence.h"
#include "PlayerItemManager.h"
#include "WeaponBase.h"
#include "WeaponController.h"
#include "Character/UserInterface/DamageDisplayWidget.h"
#include "GAS/PlayMontageAbility.h"
#include "GAS/Monster/MonsterAttackAbilityBase.h"
#include "GAS/Player/PlayerAttributeSet.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class TEAMD_API APlayerCharacter : public ACharacterBase
{
	GENERATED_BODY()

protected:
	APlayerCharacter();
	
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UUserWidget> PlayerCommonWidgetClass;

//------------------------GAS------------------------

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UPlayerAttributeSet* GetPlayerAttributeSet();

protected:
//------------------------input------------------------
	
	// MappingContext
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputAction> MoveInput;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputAction> LookInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputAction> DodgeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputAction> DashInput;

	// Ability再生用input tag

	// Dodge
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	FGameplayTag DodgeInputTag;

	// 入力保存状態のTag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	FGameplayTag SaveInputStateTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	FGameplayTag SaveInputStateTagRoot;

public:
	FGameplayTag GetSaveInputStateTag() const { return SaveInputStateTag; }

	FGameplayTag GetSaveInputStateTagRoot() const { return SaveInputStateTagRoot; }

private:
	// input設定
	void SetupInput();

	// イベントハンドラー関数
	// Move
	void MovePlayer(const FInputActionValue& Value);
	// Look
	void RotateControllerInput(const FInputActionValue& Value);
	// Dodge
	void PressedDodge();
	// Dash
	void DownDash();
	void ReleasedDash();

	//------------------------移動------------------------

	// 移動速度を状態に応じて更新する
	void UpdateMovementSpeed() const;

protected:
	// 歩く速度 CharacterMovementから初期化する
	float WalkSpeed;

	// 走る速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
	float DashSpeed;

	bool bIsDashing = false;

	// 抜刀納刀モーション中の速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
	float SheathingOrDrawingSpeed;

	// 抜刀納刀モーション中の速度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
	float UsingItemSpeed;

	// 抜刀納刀モーション中の状態Tag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
	FGameplayTag SheathingOrDrawingStateTag;

	// 抜刀納刀モーション中の状態Tag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
	FGameplayTag UsingItemStateTag;

//------------------------コンポーネント------------------------

	// 武器コントローラー
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component)
	TObjectPtr<UWeaponController> WeaponController;
	
	// アイテム管理
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component)
	TObjectPtr<UPlayerItemManager> PlayerItemManager;

public:
	TObjectPtr<UWeaponController> GetWeaponController() { return WeaponController; }

	TObjectPtr<UPlayerItemManager> GetPlayerItemManager() { return PlayerItemManager; }

	//------------------------被弾------------------------

	// 食らったダメージ情報を直接受け取る
	void OnReceiveDamage(float Damage, const FVector& DamageDirection, const TObjectPtr<UMonsterAttackAbilityBase>& AttackAbility);

	// AttributeSetからのコールバック todo:CharacterBaseでまとめたいし、名前も要相談
	UFUNCTION()
	void OnReceiveDamage(float Health);

protected:
	// 死んだとき todo:これも
	void OnDead();

	// 死んだときのAbility
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Damage)
	TSubclassOf<UGameplayAbility> PlayerDeadAbility;

	// 被弾時のAbility
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Damage)
	TArray<TSubclassOf<UPlayMontageAbility>> DamageMotions;

	// 死んだときのシーケンス
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Damage)
	TSoftObjectPtr<ULevelSequence> DeadSequence;
};
