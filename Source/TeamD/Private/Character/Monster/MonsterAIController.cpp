#include "Character/Monster/MonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"

AMonsterAIController::AMonsterAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("AIPerceptionComponent");
}

void AMonsterAIController::BeginPlay()
{
	Super::BeginPlay();

	// BehaviorTreeの設定
	if (BehaviorTree)
	{
		RunBehaviorTree(BehaviorTree);

		if (Blackboard)
		{
			// Blackboardに参照を設定する
			Blackboard->SetValueAsObject(SelfActorKeyName, GetPawn());
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("BehaviorTreeがない"));
	}

	// Perceptionの更新を受け取る
	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AMonsterAIController::OnPerceptionUpdated);
	}
}

TObjectPtr<AActor> AMonsterAIController::GetTargetActor() const
{
	return Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKeyName));
}

void AMonsterAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	for (const auto UpdatedActor : UpdatedActors)
	{
		FActorPerceptionBlueprintInfo PerceptionInfo;
		AIPerceptionComponent->GetActorsPerception(UpdatedActor, PerceptionInfo);

		// index 0 は視界
		if (PerceptionInfo.LastSensedStimuli[0].WasSuccessfullySensed())
		{
			// 視界に入ったらBlackboardに登録する
			Blackboard->SetValueAsObject(TargetActorKeyName, PerceptionInfo.Target);

			// 戦闘BGMを流す
			if (!AtomComponent && ((AtomComponent = NewObject<UAtomComponent>(this))))
			{
				AtomComponent->SetSound(BattleBGM);
				AtomComponent->Play();
			}
		}
	}
}
