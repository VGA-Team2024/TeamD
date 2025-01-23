#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "LevelSequence.h"
#include "WeaponBase.h"
#include "Character/UserInterface/DamageDisplayWidget.h"
#include "GAS/PlayMontageAbility.h"
#include "GAS/Monster/MonsterAttackAbilityBase.h"
#include "PlayerCharacter.generated.h"

// プレイヤーの装備
USTRUCT(BlueprintType)
struct FPlayerEquipmentStruct
{
	GENERATED_BODY()

	// 武器
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AWeaponBase> Weapon;
	
	FPlayerEquipmentStruct()
		: Weapon(nullptr)
	{
	}
};

UCLASS()
class TEAMD_API APlayerCharacter : public ACharacterBase
{
	GENERATED_BODY()

protected:
	APlayerCharacter();
	
	virtual void BeginPlay() override;

	// 自身のSkeletalMesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component)
	TObjectPtr<USkeletalMeshComponent> PlayerMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UUserWidget> PlayerCommonWidgetClass;

//------------------------GAS------------------------

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UPlayerAttributeSet* GetPlayerAttributeSet();

protected:
	// 攻撃のEffect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GAS)
	TSubclassOf<UGameplayEffect> DealDamageEffectClass;
	
//------------------------input------------------------
	
	// MappingContext
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputAction> MoveInput;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputAction> LookInput;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputAction> NormalAttackInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputAction> DodgeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputAction> DashInput;

	// Ability再生用input tag
	// 通常攻撃
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	FGameplayTag NormalAttackTag;
	
	// 抜刀
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	FGameplayTag DrawingSwordTag;
	
	// 納刀
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	FGameplayTag SheathingOfSwordTag;

	// Dodge
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	FGameplayTag DodgeInputTag;

	// 入力保存状態のTag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	FGameplayTag SaveInputStateTag;// = FGameplayTag::RequestGameplayTag(FName("Input.SaveInput"));
	
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
	// Attack
	void NormalAttack();
	// Dodge
	void PressedDodge();
	void ReleasedDodge();
	// Dash
	void PressedDash();
	void ReleasedDash(){};

//------------------------状態------------------------

protected:
	// 抜刀状態か todo EnumとかTagでやってもいい 納刀状態かはプレイヤーが持つか武器が持つか
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = State)
	bool IsDrawing = false;

//------------------------装備------------------------

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	FPlayerEquipmentStruct PlayerEquipment;

	// 現在装備している武器のActor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Equipment)
	TObjectPtr<AWeaponBase> WeaponActor;

protected:
	// Playerの装備から武器を適用させる
	void ApplyWeapon();

//------------------------攻撃------------------------

	// ダメージを与える
	UFUNCTION()
	void DealDamage(FHitResult HitResult);

	// ヒットストップ
	UFUNCTION()
	void AnimHitStop(FHitResult HitResult);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attack)
	float StopSpeed = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attack)
	float HitStopDuration = 0.f;

public:
	// 与えたダメージ情報を受け取る
	void OnDealtDamage(float Damage, FVector HitPoint);

	// ダメージUIのクラス
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attack)
	TSubclassOf<UDamageDisplayWidget> DamageUIClass;

	//------------------------被弾------------------------

	// 食らったダメージ情報を受け取る
	void OnReceiveDamage(float Damage, const FVector& DamageDirection, const TObjectPtr<UMonsterAttackAbilityBase>& AttackAbility);

	// AttributeSetからのコールバック todo:CharacterBaseでまとめたいし、名前も要相談
	UFUNCTION()
	void OnReceiveDamage(float Health);

	// 死んだとき todo:これも
	void OnDead();

	// 被弾時のAbility
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Damage)
	TArray<TSubclassOf<UPlayMontageAbility>> DamageMotions;

	// 死んだときのシーケンス
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Damage)
	TSoftObjectPtr<ULevelSequence> DeadSequence;
};
