#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "ItemBase.h"
#include "PlayerItemManager.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FItemInfoDelegate, TArray<TObjectPtr<UItemBase>>, int32);

/**
 * プレイヤーの所持しているアイテムを管理
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TEAMD_API UPlayerItemManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerItemManager();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY()
	TObjectPtr<UCustomAbilitySystemComponent> CustomAbilitySystemComponent;

	//------------------------Input------------------------

	// 入力初期設定
	void SetupInput();

	// MappingContext
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputMappingContext> ItemMappingContext;

	// アイテム使用InputAction
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> UseItemInput;

	// アイテム使用コールバック
	void UseItem();

	//------------------------アイテム管理------------------------

	// 所有しているアイテム
	UPROPERTY()
	TArray<TObjectPtr<UItemBase>> ItemsInPossession;
	
	// 選択しているアイテムIndex
	int32 SelectedItemIndex = 0;

public:
	// 所有しているアイテムを取得する
	const TArray<TObjectPtr<UItemBase>>& GetItemsInPossession() const { return ItemsInPossession; }

	// 所有アイテムが変更されたとき
	FItemInfoDelegate OnItemInfoChanged;

	void ItemInfoChangedBroadcast() const;

	/**
	 * アイテムを追加する
	 * @param ItemToAdd 追加するアイテムクラス
	 * @param Num 追加する個数
	 */
	UFUNCTION(BlueprintCallable, category = Item)
	void AddItem(TSubclassOf<UItemBase> ItemToAdd, int32 Num);

	// 選択のIndexを取得
	int32 GetSelectedItemIndex() const { return SelectedItemIndex; }
};
