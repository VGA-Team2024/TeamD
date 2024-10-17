#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "DealDamageCustomCalculation.generated.h"

/**
 * プレイヤーがモンスターにダメージを与えるときの計算クラス
 */
UCLASS()
class TEAMD_API UDealDamageCustomCalculation : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
