#include "Character/UserInterface/PlayerCommonWidget.h"
#include "Character/Player/PlayerCharacter.h"

void UPlayerCommonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// PlayerAttributeSetを取得
	if (const TObjectPtr<APlayerCharacter> Player = Cast<APlayerCharacter>(GetOwningPlayer()->GetPawn()))
	{
		if ((PlayerAttributeSet = Player->GetPlayerAttributeSet()))
		{
			PlayerAttributeSet->OnChangedHealth.AddDynamic(this, &UPlayerCommonWidget::OnChangedHealthEvent);
		}
		else UE_LOG(LogTemp, Warning, TEXT("UPlayerCommonWidget : Attributeがnull"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UPlayerCommonWidget : Playerを取得できなかった"));
	}
}

void UPlayerCommonWidget::OnChangedHealthEvent(float Value)
{
	HealthBar->SetPercent(Value / PlayerAttributeSet->GetMaxHealth());
}
