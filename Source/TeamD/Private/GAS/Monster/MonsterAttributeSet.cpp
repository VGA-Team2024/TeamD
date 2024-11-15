#include "GAS/Monster/MonsterAttributeSet.h"
#include "GameplayEffectExtension.h"

UMonsterAttributeSet::UMonsterAttributeSet()
	: Health(100.f)
	, MaxHealth(100.f)
{
}

void UMonsterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	UE_LOG(LogTemp, Log, TEXT("damage : %f, hp : %f"), Data.EvaluatedData.Magnitude, GetHealth());
}
