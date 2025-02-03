#include "GAS/CustomAbilitySystemComponent.h"
#include "Framework/CustomFramework.h"

void UCustomAbilitySystemComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bUseSaveInput) TryActivateAbilitiesBySavedInputTagInTick(DeltaTime);
}

FGameplayAbilitySpec UCustomAbilitySystemComponent::GiveAbilityAndActivateOnce(
	const TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!AbilityClass)
	{
		LOG_INFO(Error, "Invalid Ability Class");
		return nullptr;
	}
    
	// SpecをAbilitySystemに一時的に追加
	const FGameplayAbilitySpecHandle SpecHandle = GiveAbility(FGameplayAbilitySpec(AbilityClass));

	// Abilityを発動 発動出来なかったらAbilityを削除して終了
	if (const bool bActivated = TryActivateAbility(SpecHandle); !bActivated)
	{
		LOG_INFO(Log, "Failed to activate temporary ability");
		ClearAbility(SpecHandle);
		return nullptr;
	}

	// アクティブなAbilityインスタンス取得
	if (UGameplayAbility* ActiveAbility = GetActiveAbility(SpecHandle))
	{
		// Ability終了時に削除する処理を登録
		ActiveAbility->OnGameplayAbilityEnded.AddUObject(this, &UCustomAbilitySystemComponent::RemoveAbilityByReference);
	}
	else
	{
		LOG_INFO(Warning, "ActiveInstance is null, ability may have already ended");
		ClearAbility(SpecHandle);
	}

	// Specを返す
	return *FindAbilitySpecFromHandle(SpecHandle);
}

void UCustomAbilitySystemComponent::RemoveAbilityByReference(UGameplayAbility* Ability)
{
	ClearAbility(Ability->GetCurrentAbilitySpecHandle());
}

TArray<UGameplayAbility*> UCustomAbilitySystemComponent::GetActiveAbilities()
{
	TArray<UGameplayAbility*> ActiveAbilities;
	
	for (FGameplayAbilitySpec AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.IsActive())
		{
			ActiveAbilities.Append(AbilitySpec.GetAbilityInstances());
		}
	}

	return ActiveAbilities;
}

UGameplayAbility* UCustomAbilitySystemComponent::GetActiveAbility(const FGameplayAbilitySpecHandle SpecHandle)
{
	for (UGameplayAbility* Ability : GetActiveAbilities())
	{
		if (Ability->GetCurrentAbilitySpecHandle() == SpecHandle)
		{
			return Ability;
		}
	}

	return nullptr;
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
