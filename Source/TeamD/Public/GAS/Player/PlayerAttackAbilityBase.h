#pragma once

#include "CoreMinimal.h"
#include "GAS/PlayMontageAbility.h"
#include "PlayerAttackAbilityBase.generated.h"

/**
 * プレイヤーの攻撃Abilityの基底
 * Tagの初期化はBPでやろう
 */
UCLASS()
class TEAMD_API UPlayerAttackAbilityBase : public UPlayMontageAbility
{
	GENERATED_BODY()

public:
	// モーション値
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Parameter)
	int MotionValue;
};
