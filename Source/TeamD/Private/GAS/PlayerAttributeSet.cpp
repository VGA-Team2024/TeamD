#include "GAS/PlayerAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"

UPlayerAttributeSet::UPlayerAttributeSet()
	: Health(1.f)
	, MaxHealth(1.f)
	, Stamina(1.f)
	, MaxStamina(1.f)
	, AttackPower(1.f)
	, Defense(0.f)
{
}

void UPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	// EffectOwnerのASCを取得
	TObjectPtr<UAbilitySystemComponent> Source = Context.GetOriginalInstigatorAbilitySystemComponent();

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), .0f, GetMaxHealth()));

		// todo 変更通知どうしようかな
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), .0f, GetMaxStamina()));
	}
}
