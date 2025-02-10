#include "Character/UserInterface/DamageDisplayWidget.h"

void UDamageDisplayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TextAnimation)
	{
		PlayAnimation(TextAnimation);
	}
}

void UDamageDisplayWidget::InitDamageDisplay(const int Value, const FVector& Location, APlayerController* PlayerController)
{
	if (DamageTextBlock)
	{
		DamageTextBlock->SetText(FText::AsNumber(Value));
	}
	
	WorldLocation = Location;
	DisplayController = PlayerController;
	// Offsetを決定して保存
	RandomOffset = FVector2D(
		FMath::FRandRange(-RandomOffsetRange.X, RandomOffsetRange.X),
		FMath::FRandRange(-RandomOffsetRange.Y, RandomOffsetRange.Y));
}

void UDamageDisplayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (DisplayController)
	{
		// テキスト位置をセット
		FVector2D ScreenPosition;
		DisplayController->ProjectWorldLocationToScreen(WorldLocation, ScreenPosition);
		// offsetを追加
		ScreenPosition += RandomOffset;
		SetPositionInViewport(ScreenPosition);
	}
}

void UDamageDisplayWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);

	if (Animation == TextAnimation)
	{
		RemoveFromParent();
	}
}
