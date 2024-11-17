#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MonsterAIController.generated.h"

/**
 * モンスターのコントローラー
 */
UCLASS()
class TEAMD_API AMonsterAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMonsterAIController();
	
	virtual void BeginPlay() override;

	// 使用するBehaviorTree
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = BehaviorTree)
	TObjectPtr<UBehaviorTree> BehaviorTree;
};
