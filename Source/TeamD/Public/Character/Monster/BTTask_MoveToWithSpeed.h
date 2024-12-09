#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_MoveToWithSpeed.generated.h"

/**
 * 移動スピードを設定できるMoveTo
 */
UCLASS()
class TEAMD_API UBTTask_MoveToWithSpeed : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UBTTask_MoveToWithSpeed();
	
protected:
	// 設定する移動速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Node)
	float MoveSpeed;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
	// 元のスピード
	float OriginalSpeed;
};
