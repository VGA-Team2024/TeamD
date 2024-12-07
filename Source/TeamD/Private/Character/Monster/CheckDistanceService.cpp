#include "Character/Monster/CheckDistanceService.h"
#include "BehaviorTree/BlackboardComponent.h"

void UCheckDistanceService::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const TObjectPtr<UBlackboardComponent> BlackboardComponent = OwnerComp.GetBlackboardComponent();

	if(!BlackboardComponent) return;

	const TObjectPtr<AActor> SelfActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(SelfActorKeyName.SelectedKeyName));
	const TObjectPtr<AActor> TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKeyName.SelectedKeyName));

	if (SelfActor && TargetActor)
	{
		// 距離をSetValueする
		BlackboardComponent->SetValueAsFloat(SetDistanceKeyName.SelectedKeyName,
			FVector::Distance(SelfActor->GetActorLocation(), TargetActor->GetActorLocation()));
	}
}
