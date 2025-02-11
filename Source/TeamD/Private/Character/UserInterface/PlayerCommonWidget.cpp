#include "Character/UserInterface/PlayerCommonWidget.h"
#include "Character/Player/PlayerCharacter.h"
#include "Framework/CustomFramework.h"

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

		if ((PlayerItemManager = Player->GetPlayerItemManager()))
		{
			PlayerItemManager->OnItemInfoChanged.AddUObject(this, &UPlayerCommonWidget::OnItemInfoChanged);
			OnItemInfoChanged(PlayerItemManager->GetItemsInPossession(), PlayerItemManager->GetSelectedItemIndex());
		}
		else LOG_INFO(Warning, "PlayerItemManager null");
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

void UPlayerCommonWidget::OnItemInfoChanged(TArray<TObjectPtr<UItemBase>> ItemsInPossession, int32 SelectedIndex)
{
	ItemStackText->SetText(FText::AsNumber(ItemsInPossession[SelectedIndex]->GetCurrentStack()));
}
