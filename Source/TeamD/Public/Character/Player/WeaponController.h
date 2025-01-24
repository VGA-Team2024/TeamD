#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "InputAction.h"
//#include "InputActionValue.h" 使うかな　何に使うんだこれ
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "GAS/CustomAbilitySystemComponent.h"
#include "WeaponController.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TEAMD_API UWeaponController : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponController();

protected:
	virtual void BeginPlay() override;

	// todo:プレイヤーと武器、参照はどっちからか

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// OwnerのCharacterBaseからCustomASCを取得する
	void GetCustomASC();

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

	// 通常攻撃InputTag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	FGameplayTag NormalAttackInputTag;

	// 抜刀InputTag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	FGameplayTag DrawingSwordInputTag;

	// 通常攻撃コールバック
	void NormalAttack();

	//------------------------状態------------------------

	// 抜刀状態であるか
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = State)
	bool bIsDrawing = false;
};
