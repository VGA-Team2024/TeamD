#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Character/Player/PlayerCharacter.h"
#include "AbilityTransitionNotifyState.generated.h"

/**
 * 保存した入力から特定のAbilityに遷移できる
 */
UCLASS()
class TEAMD_API UAbilityTransitionNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;

protected:

	// 遷移できるAbility
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSubclassOf<UGameplayAbility>> TransitionalAbilities;
};
