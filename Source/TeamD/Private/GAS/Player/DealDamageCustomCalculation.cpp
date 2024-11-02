#include "GAS/Player/DealDamageCustomCalculation.h"
#include "Character/Monster/MonsterCharacter.h"
#include "Character/Player/PlayerCharacter.h"

float UDealDamageCustomCalculation::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// プレイヤー
	TObjectPtr<APlayerCharacter> OwnerPlayer = Cast<APlayerCharacter>(Spec.GetEffectContext().GetInstigator());
	// 攻撃されたモンスター
	TObjectPtr<AMonsterCharacter> TargetMonster = Cast<AMonsterCharacter>(Spec.GetEffectContext().GetInstigator());

	if (!OwnerPlayer || !TargetMonster)
	{
		UE_LOG(LogTemp, Warning, TEXT("Owner or Target is null when calculating damage"));
		return 0.f;
	}
	
	// プレイヤーの攻撃力
	float PlayerCalcedAttackPower = OwnerPlayer->WeaponActor->WeaponStatus.AttackPower;
	
	return 10.f;
}
