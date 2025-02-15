#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "GAS/Player/ItemAbilityBase.h"
#include "ItemBase.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(OneParamDelegate, int32);

/**
 * プレイヤーが使用するアイテムの基底
 */
UCLASS(Blueprintable)
class TEAMD_API UItemBase : public UObject
{
	GENERATED_BODY()

public:
	// 初期化
	void Init(const TObjectPtr<UCustomAbilitySystemComponent>& CustomAbilitySystemComponent);

	UPROPERTY()
	TObjectPtr<UCustomAbilitySystemComponent> OwnerCustomASC;

	//------------------------ItemAction------------------------
	
	// アイテム使用のAbility再生用
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TSubclassOf<UItemAbilityBase> ItemAbilityClass;
	
	// アイテムの使用
	virtual void UseItem();

	// アイテムの効果
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ApplyItemEffect();

	// todo:上に同じ
	virtual void ApplyItemEffect_Implementation();

	// アイテムの追加
	void AddStack(int32 Value);

	// アイテムが消費されたとき
	OneParamDelegate OnItemStackChanged;

	//------------------------ItemProperty------------------------
	
	// 最大スタック数
	UFUNCTION(BlueprintPure)
	int32 GetMaxStack() const;

	// 現在のスタック数
	UFUNCTION(BlueprintPure)
	int32 GetCurrentStack() const;

protected:
	// 最大スタック数
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxStackNum = 10;

	// 現在のスタック数
	int32 CurrentStackNum = 0;
};
