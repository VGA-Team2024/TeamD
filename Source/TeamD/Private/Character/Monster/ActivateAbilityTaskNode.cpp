#include "Character/Monster/ActivateAbilityTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/CharacterBase.h"
#include "GAS/Monster/MonsterAttackAbilityBase.h"

EBTNodeResult::Type UActivateAbilityTaskNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Blackboardの取得
	const TObjectPtr<UBlackboardComponent> BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return EBTNodeResult::Failed;

	// SelfCharacterの取得
	const TObjectPtr<ACharacterBase> SelfCharacter = Cast<ACharacterBase>(BlackboardComp->GetValueAsObject(SelfActorKey.SelectedKeyName));
	if (!SelfCharacter) return EBTNodeResult::Failed;

	// Abilityを起動
	if (const TObjectPtr<UAbilitySystemComponent> SelfASC = SelfCharacter->GetAbilitySystemComponent())
	{
		if (!SelfASC->TryActivateAbilityByClass(ActivateAbility))
		{
			return EBTNodeResult::Failed;
		}
	}
	else return EBTNodeResult::Failed;

	TArray<UGameplayAbility*> ActiveAbilities;
	SelfCharacter->GetActiveAbilities(ActiveAbilities);

	// 現在再生中の攻撃Abilityを取得してイベントに登録する
	for (const auto Ability : ActiveAbilities)
	{
		// MonsterAttackにCast モンスター以外にも使う場合、新しく基底を作る
		if (const TObjectPtr<UMonsterAttackAbilityBase> MonsterAbility = Cast<UMonsterAttackAbilityBase>(Ability))
		{
			MonsterAbility->OnEndAbility.AddDynamic(this, &UActivateAbilityTaskNode::OnEndAbility);
			OwnerBTComp = OwnerComp;
			
			// 一つ見つけたら終了 Taskの終了はしない
			return EBTNodeResult::InProgress;
		}
	}

	return EBTNodeResult::Failed;
}

void UActivateAbilityTaskNode::OnEndAbility()
{
	FinishLatentTask(*OwnerBTComp, EBTNodeResult::Succeeded);
}

FString UActivateAbilityTaskNode::GetStaticDescription() const
{
	return FString::Printf(TEXT("ActivateAbility : %s"), *ActivateAbility->GetName());
}
