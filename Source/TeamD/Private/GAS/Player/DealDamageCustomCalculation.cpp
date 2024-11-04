#include "GAS/Player/DealDamageCustomCalculation.h"
#include "Character/Monster/MonsterCharacter.h"
#include "Character/Player/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "GAS/Player/PlayerAttackAbilityBase.h"

float UDealDamageCustomCalculation::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// プレイヤー
	const TObjectPtr<APlayerCharacter> OwnerPlayer = Cast<APlayerCharacter>(Spec.GetEffectContext().GetInstigator());
	// 攻撃されたモンスター
	TObjectPtr<AMonsterCharacter> TargetMonster;

	if (Spec.GetEffectContext().GetHitResult())
	{
		TargetMonster = Cast<AMonsterCharacter>(Spec.GetEffectContext().GetHitResult()->GetActor());
	}

	TObjectPtr<UAbilitySystemComponent> PlayerAbilitySystem;

	if (!OwnerPlayer || !TargetMonster || !((PlayerAbilitySystem = OwnerPlayer->GetAbilitySystemComponent())))
	{
		UE_LOG(LogTemp, Warning, TEXT("ダメージ計算時になんかnull"));
		return 0.f;
	}
	
	// 計算済みのダメージ値 初期値にプレイヤーの攻撃力を入れる 調整で100で割る todo プレイヤーの一時的なバフを追加する
	float CalculatedDamage = OwnerPlayer->WeaponActor->WeaponStatus.AttackPower / 100;

	// アビリティを取得
	const TObjectPtr<UGameplayAbility> ActiveAbility = PlayerAbilitySystem->GetActivatableAbilities()[0].Ability;
	TObjectPtr<UPlayerAttackAbilityBase> AttackAbility;

	// 攻撃アビリティからモーション値を掛ける
	if (ActiveAbility && ((AttackAbility = Cast<UPlayerAttackAbilityBase>(ActiveAbility))))
	{
		CalculatedDamage *= AttackAbility->MotionValue;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ダメージ計算時に攻撃アビリティを確認できなかった"));
		return 0.f;
	}

	// todo 会心、状態補正、肉質
	
	return CalculatedDamage;
}
