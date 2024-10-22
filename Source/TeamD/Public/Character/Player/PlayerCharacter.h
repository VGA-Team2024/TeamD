#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "WeaponBase.h"
#include "GAS/Player/PlayerAttributeSet.h"
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

//------------------------GAS------------------------

	UPROPERTY(BlueprintReadOnly, Category = GAS)
	TObjectPtr<UPlayerAttributeSet> PlayerAttributeSet;
	
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

	// input tag
	// 通常攻撃
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	FGameplayTagContainer NormalAttackTag;

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

//------------------------装備------------------------

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	FPlayerEquipmentStruct PlayerEquipment;

	// 現在装備している武器のActor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Equipment)
	TObjectPtr<AWeaponBase> WeaponActor;

	// Playerの装備から武器を適用させる
	void ApplyWeapon();

//------------------------攻撃------------------------

	// ダメージを与える
	UFUNCTION()
	void DealDamage(AActor* Target);
};
