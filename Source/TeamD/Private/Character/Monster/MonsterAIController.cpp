#include "Character/Monster/MonsterAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/LogMacros.h"

AMonsterAIController::AMonsterAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMonsterAIController::BeginPlay()
{
	Super::BeginPlay();

	if (BehaviorTree)
	{
		RunBehaviorTree(BehaviorTree);

		if (Blackboard)
		{
			Blackboard->SetValueAsObject("TargetActor", UGameplayStatics::GetPlayerPawn(this, 0));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("BehaviorTreeがない"));
	}
}
