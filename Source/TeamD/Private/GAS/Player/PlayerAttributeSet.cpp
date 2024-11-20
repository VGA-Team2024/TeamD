#include "GAS/Player/PlayerAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"

UPlayerAttributeSet::UPlayerAttributeSet()
	: Health(100.f)
	, MaxHealth(100.f)
	, Stamina(100.f)
	, MaxStamina(100.f)
	, AttackPower(0.f)
	, Defense(1.f)
{
}

void UPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	// EffectOwnerのASCを取得
	TObjectPtr<UAbilitySystemComponent> Source = Context.GetOriginalInstigatorAbilitySystemComponent();

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), .0f, GetMaxHealth()));
		OnChangedHealth.Broadcast(GetHealth());
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), .0f, GetMaxStamina()));
	}
}
