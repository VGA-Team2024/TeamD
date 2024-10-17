#include "GAS/Monster/MonsterAttributeSet.h"

UMonsterAttributeSet::UMonsterAttributeSet()
	: Health(1.f)
	, MaxHealth(1.f)
{
}

void UMonsterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	UE_LOG(LogTemp, Log, TEXT("hp : %f"), GetHealth());
}
