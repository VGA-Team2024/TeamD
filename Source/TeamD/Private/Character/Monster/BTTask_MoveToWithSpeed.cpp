#include "Character/Monster/BTTask_MoveToWithSpeed.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_MoveToWithSpeed::UBTTask_MoveToWithSpeed(): MoveSpeed(0), OriginalSpeed(0)
{
	NodeName = "Move To With Speed";
}

EBTNodeResult::Type UBTTask_MoveToWithSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const TObjectPtr<AAIController> AIController = OwnerComp.GetAIOwner();

	if (!AIController) return EBTNodeResult::Failed;

	// CharacterMovementのスピードを設定する
	if (const TObjectPtr<ACharacter> OwnerCharacter = Cast<ACharacter>(AIController->GetPawn()))
	{
		OriginalSpeed = OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed;
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	}

	// Move Toの処理を呼ぶ
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}

void UBTTask_MoveToWithSpeed::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	const TObjectPtr<AAIController> AIController = OwnerComp.GetAIOwner();

	if (!AIController) return;

	// 元の速度に戻す
	if (const TObjectPtr<ACharacter> OwnerCharacter = Cast<ACharacter>(AIController->GetPawn()))
	{
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = OriginalSpeed;
	}
	
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
