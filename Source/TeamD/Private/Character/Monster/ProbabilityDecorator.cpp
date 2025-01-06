#include "Character/Monster/ProbabilityDecorator.h"

bool UProbabilityDecorator::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return Percentage / 100.f >= FMath::FRand();
}
