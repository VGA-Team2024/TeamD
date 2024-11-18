#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MonsterAttackNotifyState.generated.h"

/**
 * モンスターの攻撃当たり判定を切り替えるAnimState
 */
UCLASS()
class TEAMD_API UMonsterAttackNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	// 攻撃判定有効にするPhysicsAssetのShapeのBoneName
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> EnableShapesBoneName;
};
