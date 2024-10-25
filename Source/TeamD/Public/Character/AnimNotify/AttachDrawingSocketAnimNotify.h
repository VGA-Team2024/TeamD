#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AttachDrawingSocketAnimNotify.generated.h"

/**
 * プレイヤーの武器を納刀状態のソケットにアタッチする
 */
UCLASS()
class TEAMD_API UAttachDrawingSocketAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
