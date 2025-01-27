#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "ItemBase.h"
#include "PlayerItemManager.generated.h"

/**
 * プレイヤーの所持しているアイテムを管理
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TEAMD_API UPlayerItemManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerItemManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//------------------------Input------------------------
protected:
	// 入力初期設定
	void SetupInput();

	// MappingContext
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputMappingContext> ItemMappingContext;

	// 通常攻撃
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> UseItemInput;

	// アイテム使用コールバック
	void UseItem();

	//------------------------アイテム管理------------------------

	// 所有しているアイテム
	TArray<TObjectPtr<UItemBase>> ItemsInPossession;

public:
	// 所有しているアイテムを取得する
	UFUNCTION(BlueprintPure, Category = Item)
	const TArray<UItemBase*>& GetItemsInPossession() const { return ItemsInPossession; }

	/**
	 * アイテムを追加する
	 * @param ItemToAdd 追加するアイテムクラス
	 * @param Num 追加する個数
	 */
	UFUNCTION(BlueprintCallable, category = Item)
	void AddItem(TSubclassOf<UItemBase> ItemToAdd, int32 Num);

protected:
	// 選択しているアイテムIndex
	int32 SelectedItems = 0;
};
