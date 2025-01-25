#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "EquipmentStruct.generated.h"

/**
 * プレイヤーの装備の構造体
 */
USTRUCT(BlueprintType)
struct FEquipmentStruct
{
	GENERATED_BODY()

	// 武器
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AWeaponBase> Weapon;

	// 他の装備とかなさそうだけどね
	
	FEquipmentStruct()
		: Weapon(nullptr)
	{
	}
};
