#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "DamageCustomCalculation.generated.h"

/**
 * モンスターから受けるダメージの計算クラス
 */
UCLASS()
class TEAMD_API UDamageCustomCalculation : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
