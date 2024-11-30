#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "BehaviorTree/BTTaskNode.h"
#include "ActivateAbilityTaskNode.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class TEAMD_API UActivateAbilityTaskNode : public UBTTaskNode
{
	GENERATED_BODY()

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// AbilityのコールバックからTaskを終了する
	UFUNCTION()
	void OnEndAbility();

	virtual FString GetStaticDescription() const override;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector SelfActorKey;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayAbility> ActivateAbility;

private:
	TObjectPtr<UBehaviorTreeComponent> OwnerBTComp;
};
