#include "GAS/Monster/MonsterAttackAbilityBase.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

void UMonsterAttackAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                const FGameplayEventData* TriggerEventData)
{
	if (!MontageToPlay) return;

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	// アニメーションタスクの作成
	const TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, MontageToPlay);

	// アニメーション終了のコールバックにアビリティ終了を登録
	MontageTask->OnCompleted.AddDynamic(this, &UMonsterAttackAbilityBase::CallEndAbility);
	MontageTask->OnBlendOut.AddDynamic(this, &UMonsterAttackAbilityBase::CallEndAbility);
	MontageTask->OnInterrupted.AddDynamic(this, &UMonsterAttackAbilityBase::CallEndAbility);
	MontageTask->OnCancelled.AddDynamic(this, &UMonsterAttackAbilityBase::CallEndAbility);

	// タスクの再生
	MontageTask->ReadyForActivation();
}

void UMonsterAttackAbilityBase::CallEndAbility()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
	OnEndAbility.Broadcast();
}
