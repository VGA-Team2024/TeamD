#include "GAS/CustomAbilitySystemComponent.h"

void UCustomAbilitySystemComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TryActivateAbilitiesBySavedInputTagInTick(DeltaTime);
}

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

void UCustomAbilitySystemComponent::SaveTagTryActivateAbilities(const FGameplayTag InputTag)
{
	if (!InputTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid InputTag"));
		return;
	}
	
	// Activate出来なかった場合
	if (!TryActivateAbilitiesByTag(FGameplayTagContainer(InputTag), true))
	{
		SavedInputTag = InputTag;
		// タイマーリセット
		SaveInputTimer = TimeToSaveInput;
	}
}

void UCustomAbilitySystemComponent::TryActivateAbilitiesBySavedInputTagInTick(float DeltaTime)
{
	if (SaveInputTimer <= 0) return;

	if (SavedInputTag.IsValid() && TryActivateAbilitiesByTag(FGameplayTagContainer(SavedInputTag), true))
	{
		// タイマーリセットして終了
		SaveInputTimer = 0;
		return;
	}

	// タイマーを進める
	SaveInputTimer -= DeltaTime;
}
