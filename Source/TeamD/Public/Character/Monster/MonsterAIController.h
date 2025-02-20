#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Atom/AtomSoundBase.h"
#include <Atom/AtomComponent.h>
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
	TObjectPtr<UBehaviorTree> BehaviorTree;// 感知更新時に呼ばれる関数

	// BlackboardからTargetActorを取得する
	TObjectPtr<AActor> GetTargetActor() const;
	
protected:
	// Perceptionの更新を受け取る
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	// SetValueKeyNames

	// 自分
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = KeyName)
	FName SelfActorKeyName;

	// ターゲットActor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = KeyName)
	FName TargetActorKeyName;

	// AIPerception
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AIPerception)
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY()
	TObjectPtr<UAtomComponent> AtomComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Cri)
	TObjectPtr<UAtomSoundBase> BattleBGM;
};
