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
	float CalculatedDamage = OwnerPlayer->GetWeaponController()->GetWeaponActor()->WeaponStatus.AttackPower / 100.f;

	// アビリティを取得
	TObjectPtr<UPlayerAttackAbilityBase> AttackAbility;

	for (FGameplayAbilitySpec AbilitySpec : PlayerAbilitySystem->GetActivatableAbilities())
	{
		if (AbilitySpec.IsActive() && ((AttackAbility = Cast<UPlayerAttackAbilityBase>(AbilitySpec.Ability))))
		{
			break;
		}
	}

	if (!AttackAbility)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDealDamageCustomCalculation.cpp : 攻撃アビリティを取得できなかった"));
		return 0.f;
	}

	// モーション値
	CalculatedDamage *= AttackAbility->MotionValue;

	// 肉質
	CalculatedDamage *= TargetMonster->ConvertBoneNameToPart(Spec.GetEffectContext().GetHitResult()->BoneName)->MeatQuality / 100.f;

	// todo 会心、状態補正

	// 四捨五入して出力
	CalculatedDamage = std::round(CalculatedDamage);
	// Playerに与えたダメージを通知
	OwnerPlayer->GetWeaponController()->OnDealtDamage(CalculatedDamage, Spec.GetEffectContext().GetHitResult()->ImpactPoint);
	
	return CalculatedDamage;
}
