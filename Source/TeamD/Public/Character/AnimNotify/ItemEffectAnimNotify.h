#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ItemEffectAnimNotify.generated.h"

/**
 * アイテム使用のMontageからItemの効果を呼び出す
 */
UCLASS()
class TEAMD_API UItemEffectAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
