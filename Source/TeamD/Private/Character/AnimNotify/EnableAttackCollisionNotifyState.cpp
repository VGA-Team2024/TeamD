#include "Character/AnimNotify/EnableAttackCollisionNotifyState.h"

void UEnableAttackCollisionNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                    float TotalDuration)
{
	if ((OwnerPlayer = Cast<APlayerCharacter>(MeshComp->GetOwner())))
	{
		OwnerPlayer->GetWeaponController()->GetWeaponActor()->BeginWeaponAttack();
	}
}

void UEnableAttackCollisionNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (OwnerPlayer)
	{
		OwnerPlayer->GetWeaponController()->GetWeaponActor()->EndWeaponAttack();
	}
}
