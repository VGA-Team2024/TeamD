#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Character/Player/PlayerCharacter.h"
#include "SaveInputNotifyState.generated.h"

/**
 * 入力を保存する
 */
UCLASS()
class TEAMD_API USaveInputNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:
	//inline static const FGameplayTag SaveInputStateTag = FGameplayTag::RequestGameplayTag(FName("Input.SaveInput"));
};
