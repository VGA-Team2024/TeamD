#include "GAS/PlayMontageAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

void UPlayMontageAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
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
	MontageTask->OnCompleted.AddDynamic(this, &UPlayMontageAbility::CallEndAbility);
	MontageTask->OnBlendOut.AddDynamic(this, &UPlayMontageAbility::CallEndAbility);
	MontageTask->OnInterrupted.AddDynamic(this, &UPlayMontageAbility::CallEndAbility);
	MontageTask->OnCancelled.AddDynamic(this, &UPlayMontageAbility::CallEndAbility);

	// タスク開始
	MontageTask->ReadyForActivation();
}

void UPlayMontageAbility::CallEndAbility()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}