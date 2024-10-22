#include "GAS/Player/DealDamageCustomCalculation.h"

float UDealDamageCustomCalculation::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	UE_LOG(LogTemp, Log, TEXT("calc value"));
	
	return 10.f;
}
