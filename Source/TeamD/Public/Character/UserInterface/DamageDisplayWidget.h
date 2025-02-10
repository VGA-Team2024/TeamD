#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "DamageDisplayWidget.generated.h"

/**
 * 
 */
UCLASS()
class TEAMD_API UDamageDisplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	/**
	 * UI初期化処理
	 * @param Value 表示する値
	 * @param Location 攻撃を与えた位置
	 * @param PlayerController Display上の座標にするために必要
	 */
	UFUNCTION(BlueprintCallable)
	void InitDamageDisplay(int Value, const FVector& Location, APlayerController* PlayerController);

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;
	
	// TextBlock ウィジェットを参照するための変数
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DamageTextBlock;
	
	// テキストのアニメーション
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* TextAnimation;

	// 表示するワールド座標
	FVector WorldLocation;

	// 表示するPlayerController
	UPROPERTY()
	TObjectPtr<APlayerController> DisplayController;

	// 表示位置のランダムなOffsetの範囲
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DamageDisplay")
	FVector2D RandomOffsetRange;

	// ランダムOffset保存用
	FVector2D RandomOffset;
};
