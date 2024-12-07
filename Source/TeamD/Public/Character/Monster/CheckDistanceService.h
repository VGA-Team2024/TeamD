#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "CheckDistanceService.generated.h"

/**
 * Actorどうしの距離を求める
 */
UCLASS()
class TEAMD_API UCheckDistanceService : public UBTService
{
	GENERATED_BODY()

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Blackboard)
	FBlackboardKeySelector SelfActorKeyName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Blackboard)
	FBlackboardKeySelector TargetActorKeyName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Blackboard)
	FBlackboardKeySelector SetDistanceKeyName;
};
