#pragma once

#include "CoreMinimal.h"
#include "GAS/PlayMontageAbility.h"
#include "ItemAbilityBase.generated.h"

DECLARE_MULTICAST_DELEGATE(FAction);

/**
 * アイテムモーション用のAbility
 */
UCLASS()
class TEAMD_API UItemAbilityBase : public UPlayMontageAbility
{
	GENERATED_BODY()

public:
	// 効果発動時のイベント
	FAction OnActivateEffect;
};
