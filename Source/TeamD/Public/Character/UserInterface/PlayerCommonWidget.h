#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/Player/PlayerItemManager.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GAS/Player/PlayerAttributeSet.h"
#include "PlayerCommonWidget.generated.h"

/**
 * 
 */
UCLASS()
class TEAMD_API UPlayerCommonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// Health変更時のイベント
	UFUNCTION()
	void OnChangedHealthEvent(float Value);

	// アイテム情報変更時のイベント
	void OnItemInfoChanged(TArray<TObjectPtr<UItemBase>> ItemsInPossession, int32 SelectedIndex);

protected:
	// 取得したPlayerAttribute
	UPROPERTY()
	TObjectPtr<UPlayerAttributeSet> PlayerAttributeSet;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY()
	TObjectPtr<UPlayerItemManager> PlayerItemManager;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemStackText; 
};
