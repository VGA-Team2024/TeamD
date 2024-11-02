#include "GAS/Player/PlayerAttackAbilityBase.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

void UPlayerAttackAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!MontageToPlay) return;
	
	// 発動条件をチェックする
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	// タスクの作成
	const TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None,MontageToPlay);

	// コールバックを設定
	MontageTask->OnCompleted.AddDynamic(this, &UPlayerAttackAbilityBase::CallEndAbility);
	MontageTask->OnBlendOut.AddDynamic(this, &UPlayerAttackAbilityBase::CallEndAbility);
	MontageTask->OnInterrupted.AddDynamic(this, &UPlayerAttackAbilityBase::CallEndAbility);
	MontageTask->OnCancelled.AddDynamic(this, &UPlayerAttackAbilityBase::CallEndAbility);

	// タスク開始
	MontageTask->ReadyForActivation();
}

void UPlayerAttackAbilityBase::CallEndAbility()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}
