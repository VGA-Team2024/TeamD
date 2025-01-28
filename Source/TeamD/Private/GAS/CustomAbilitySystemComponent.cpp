#include "GAS/CustomAbilitySystemComponent.h"

void UCustomAbilitySystemComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bUseSaveInput) TryActivateAbilitiesBySavedInputTagInTick(DeltaTime);
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
		UE_LOG(LogTemp, Error, TEXT("Invalid InputTag:%d"), __LINE__);
		return;
	}
	
	// Activate出来なかった場合
	if (!TryActivateAbilitiesByTag(FGameplayTagContainer(InputTag), true))
	{
		SavedInputTag = InputTag;
		// タイマーリセット
		SaveInputTimer = TimeToSaveInput;
		// 入力Tagを上書き
		RemoveInputTags();
		AddLooseGameplayTag(InputTag);
	}
}

void UCustomAbilitySystemComponent::TryActivateAbilitiesBySavedInputTagInTick(float DeltaTime)
{
	if (SaveInputTimer <= 0 ||
		(SavedInputTag.IsValid() && TryActivateAbilitiesByTag(FGameplayTagContainer(SavedInputTag), true)))
	{
		// タイマーリセット
		SaveInputTimer = 0;
		// Tagをリセット
		SavedInputTag = FGameplayTag::EmptyTag;
		// InputTagを削除
		RemoveTagsWithParent(InputTagRoot);
		
		return;
	}

	// タイマーを進める
	SaveInputTimer -= DeltaTime;
}

void UCustomAbilitySystemComponent::RemoveTagsWithParent(const FGameplayTag& ParentTag)
{
	if (!ParentTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid InputTag:%s"), *GetOwner()->GetName());
		return;
	}

	// 現在のすべてのアクティブなタグを取得
	FGameplayTagContainer ActiveTags;
	GetOwnedGameplayTags(ActiveTags);

	// 親タグ以下のすべてのタグを探して全て削除
	for (const FGameplayTag& Tag : ActiveTags)
	{
		if (Tag.MatchesTag(ParentTag)) // 親タグと一致するかチェック
		{
			SetLooseGameplayTagCount(Tag, 0);
		}
	}
}

void UCustomAbilitySystemComponent::RemoveInputTags() { RemoveTagsWithParent(InputTagRoot); }
