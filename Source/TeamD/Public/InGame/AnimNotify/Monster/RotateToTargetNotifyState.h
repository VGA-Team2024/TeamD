#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "RotateToTargetNotifyState.generated.h"

/**
 * モンスターがモーション中にターゲットに向く
 * つまり攻撃がホーミングする
 */
UCLASS()
class TEAMD_API URotateToTargetNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;

protected:
	void RotateToTarget(const TObjectPtr<AActor>& OwnerActor, const TObjectPtr<AActor>& TargetActor, float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RotateSpeed;
};
