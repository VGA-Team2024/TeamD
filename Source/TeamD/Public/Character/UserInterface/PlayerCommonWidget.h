#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
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

protected:
	// 取得したPlayerAttribute
	TObjectPtr<UPlayerAttributeSet> PlayerAttributeSet;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;
};
