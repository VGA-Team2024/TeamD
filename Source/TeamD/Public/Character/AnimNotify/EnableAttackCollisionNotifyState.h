#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Character/Player/PlayerCharacter.h"
#include "EnableAttackCollisionNotifyState.generated.h"

/**
 * プレイヤーの武器の攻撃当たり判定を有効にする
 */
UCLASS()
class TEAMD_API UEnableAttackCollisionNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	TObjectPtr<APlayerCharacter> OwnerPlayer;
};
