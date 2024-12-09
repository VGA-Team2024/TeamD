#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RotateToTarget.generated.h"

/**
 * ターゲットの方向を向くTask
 */
UCLASS()
class TEAMD_API UBTTask_RotateToTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_RotateToTarget();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	// 回転スピード
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Node)
	float RotateSpeed;

	// 許容
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Node)
	float Tolerance;
	
	// 向くターゲット
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Blackboard)
	FBlackboardKeySelector TargetObject;

private:
	// 操作対象のPawn
	TObjectPtr<APawn> OwnerPawn;
	
	// 目標回転値
	FRotator RotationTargetValue;
};
