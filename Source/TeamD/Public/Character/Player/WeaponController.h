#pragma once

#include "CoreMinimal.h"
#include "EquipmentStruct.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "GameplayTagContainer.h"
#include "WeaponBase.h"
#include "Character/UserInterface/DamageDisplayWidget.h"
#include "Components/ActorComponent.h"
#include "GAS/CustomAbilitySystemComponent.h"
#include "WeaponController.generated.h"

/**
 * 武器を管理するコンポーネント
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TEAMD_API UWeaponController : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponController();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 色んな参照を一括に取得する
	void GetReference();

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;
	
	UPROPERTY()
	TObjectPtr<UCustomAbilitySystemComponent> CustomAbilitySystemComponent;

	//------------------------Input------------------------
protected:
	// 入力初期設定
	void SetupInput();

	// MappingContext
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputMappingContext> WeaponMappingContext;

	// 通常攻撃
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> NormalAttackInput;

	// todo:特殊攻撃

	// 納刀
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> SheathingOfSwordInput;

	// 通常攻撃InputTag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	FGameplayTag NormalAttackInputTag;

	// 抜刀InputTag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	FGameplayTag DrawingSwordInputTag;
	
	// 納刀InputTag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	FGameplayTag SheathingOfSwordInputTag;

	// 通常攻撃コールバック
	void NormalAttack();

	// 納刀コールバック
	void SheathingOfSword();

	//------------------------武器アクション------------------------
public:
	// 抜刀
	UFUNCTION(BlueprintCallable, Category = Action)
	void DrawingWeapon();

	// 納刀
	UFUNCTION(BlueprintCallable, Category = Action)
	void SheathingWeapon();

protected:
	/**
	 * 武器をソケットにアタッチして、Pivotを基に回転させる
	 * @param SkeletalMeshComponent アタッチするSkeletalMesh
	 * @param AttachSocketName アタッチするソケットの名前
	 * @param PivotTransform PivotのTransform
	 */
	void AttachWeaponToMesh(USkeletalMeshComponent* SkeletalMeshComponent, const FName AttachSocketName, const USceneComponent* PivotComponent) const;

	//------------------------攻撃処理------------------------

	// 攻撃のEffect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GAS)
	TSubclassOf<UGameplayEffect> DealDamageEffectClass;
	
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
	void OnDealtDamage(float Damage, FVector HitPoint);// todo:処理の移動

	// ダメージUIのクラス
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attack)
	TSubclassOf<UDamageDisplayWidget> DamageUIClass;
	
	//------------------------装備------------------------
protected:
	// Playerの装備から武器を適用させる
	void ApplyEquipment();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	FEquipmentStruct Equipment;

	// 現在装備している武器のActor
	UPROPERTY(BlueprintReadOnly, Category = Equipment)
	TObjectPtr<AWeaponBase> WeaponActor;

public:
	AWeaponBase* GetWeaponActor() { return WeaponActor; }

	//------------------------状態------------------------
	
protected:
	// 抜刀状態であるか
	UPROPERTY(BlueprintReadOnly, Category = WeaponState)
	bool bIsDrawing = false;

	// 納刀状態のTag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = WeaponState)
	FGameplayTag WeaponSheathedStateTag;

	// 抜刀状態のTag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = WeaponState)
	FGameplayTag WeaponDrawnStateTag;

public:
	bool GetIsDrawing() const { return bIsDrawing; }
};
