#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerStatusPrimaryDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class TEAMD_API UPlayerStatusPrimaryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 体力
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Health;

	// スタミナ
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Stamina;

	// 攻撃力
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float AttackPower;

	// 防御力
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Defense;
};
