#include "Character/Monster/BTTask_RotateToTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_RotateToTarget::UBTTask_RotateToTarget(): RotateSpeed(0)
{
	NodeName = "Rotate To Target";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_RotateToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (const TObjectPtr<AAIController> AIController = OwnerComp.GetAIOwner())
	{
		if (AIController)
		{
			if (!((OwnerPawn = AIController->GetPawn()))) return EBTNodeResult::Failed;
		}
		else return EBTNodeResult::Failed;
	}

	// 向くターゲット
	const TObjectPtr<AActor> TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetObject.SelectedKeyName));
	if (!TargetActor) return EBTNodeResult::Failed;

	// 方向ベクトル
	FVector DirectionToRotate = TargetActor->GetActorLocation() - OwnerPawn->GetActorLocation();
	DirectionToRotate.Z = 0;
	DirectionToRotate = DirectionToRotate.GetSafeNormal();

	RotationTargetValue = DirectionToRotate.Rotation();

	// タスク継続
	return EBTNodeResult::InProgress;
}

void UBTTask_RotateToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!OwnerPawn) FinishLatentTask(OwnerComp, EBTNodeResult::Aborted);
	
	const FRotator CurrentRotation = OwnerPawn->GetActorRotation();
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, RotationTargetValue, DeltaSeconds, RotateSpeed);

	OwnerPawn->SetActorRotation(NewRotation);

	// 向きがほぼ一致したらTaskを終了する
	if (NewRotation.Equals(RotationTargetValue, Tolerance))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

void UBTTask_RotateToTarget::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
