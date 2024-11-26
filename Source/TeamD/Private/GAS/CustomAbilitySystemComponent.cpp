#include "GAS/CustomAbilitySystemComponent.h"

FGameplayAbilitySpecHandle UCustomAbilitySystemComponent::GiveAbilityAndActivateOnce(
	TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!AbilityClass) return FGameplayAbilitySpecHandle();

	// Abilityを付与する
	const FGameplayAbilitySpec AbilitySpec(AbilityClass);
	const FGameplayAbilitySpecHandle SpecHandle = GiveAbility(AbilitySpec);

	// Abilityを起動する
	TryActivateAbility(SpecHandle);

	// Abilityが終了したときの処理を登録する
	AbilitySpec.Ability->OnGameplayAbilityEnded.AddUObject(this, &UCustomAbilitySystemComponent::OnAbilityEnded);

	return SpecHandle;
}

void UCustomAbilitySystemComponent::RemoveAbilityByClass(TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!AbilityClass) return;

	// 一致するAbilityを探す
	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability.GetClass() == AbilityClass)
		{
			// 削除
			ClearAbility(Spec.Handle);
			return;
		}
	}
}

void UCustomAbilitySystemComponent::OnAbilityEnded(UGameplayAbility* Ability)
{
	RemoveAbilityByClass(Ability->GetClass());
}
