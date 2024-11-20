#include "GAS/Monster/DamageCustomCalculation.h"
#include "Character/Monster/MonsterCharacter.h"
#include "Character/Player/PlayerCharacter.h"
#include "GAS/Monster/MonsterAttackAbilityBase.h"
#include "Kismet/GameplayStatics.h"

float UDamageCustomCalculation::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// 攻撃したモンスター
	const TObjectPtr<AMonsterCharacter> OwnerMonster = Cast<AMonsterCharacter>(Spec.GetEffectContext().GetInstigator());
	// 攻撃されたプレイヤー ゴリ押し取得
	TObjectPtr<APlayerCharacter> TargetPlayer = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(OwnerMonster->GetWorld(), 0));

	// if (Spec.GetContext().GetSourceObject()) todo TargetPlayerを取得できない　謎が深い
	// {
	// 	UE_LOG(LogTemp, Log, TEXT("%s"), *Spec.GetContext().GetSourceObject()->GetName());
	// 	TargetPlayer = Cast<APlayerCharacter>(Spec.GetContext().GetSourceObject());
	// }

	TObjectPtr<UAbilitySystemComponent> MonsterAbilitySystem;

	if (!OwnerMonster || !TargetPlayer || !((MonsterAbilitySystem = OwnerMonster->GetAbilitySystemComponent())))
	{
		UE_LOG(LogTemp, Warning, TEXT("null ref : %hd, %hd, %hd"), (OwnerMonster != nullptr), (TargetPlayer != nullptr), (MonsterAbilitySystem != nullptr));
		return 0.f;
	}

	TObjectPtr<UMonsterAttackAbilityBase> AttackAbility;

	for (FGameplayAbilitySpec AbilitySpec : MonsterAbilitySystem->GetActivatableAbilities())
	{
		if (AbilitySpec.Ability->IsActive() && ((AttackAbility = Cast<UMonsterAttackAbilityBase>(AbilitySpec.Ability))))
		{
			break;
		}
	}

	if (!AttackAbility)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDealDamageCustomCalculation.cpp : 攻撃アビリティを取得できなかった"));
		return 0.f;
	}
	
	// モンスターからのダメージはモンスター側からの値は一旦モーション値だけ 怒り補正とかあるだろうけど
	float CalculatedDamage = AttackAbility->MotionValue;

	// todo プレイヤー側の計算
	
	return CalculatedDamage;
}
