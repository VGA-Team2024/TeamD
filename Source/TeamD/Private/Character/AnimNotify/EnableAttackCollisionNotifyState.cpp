#include "Character/AnimNotify/EnableAttackCollisionNotifyState.h"

void UEnableAttackCollisionNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                    float TotalDuration)
{
	if ((OwnerPlayer = Cast<APlayerCharacter>(MeshComp->GetOwner())))
	{
		OwnerPlayer->WeaponActor->BeginWeaponAttack();
	}
}

void UEnableAttackCollisionNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (OwnerPlayer)
	{
		OwnerPlayer->WeaponActor->EndWeaponAttack();
	}
}
