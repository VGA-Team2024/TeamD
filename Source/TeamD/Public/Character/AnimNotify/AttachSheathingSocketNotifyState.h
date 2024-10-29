#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AttachSheathingSocketNotifyState.generated.h"

/**
 * 納刀状態のソケットに武器をアタッチする
 */
UCLASS()
class TEAMD_API UAttachSheathingSocketNotifyState : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
