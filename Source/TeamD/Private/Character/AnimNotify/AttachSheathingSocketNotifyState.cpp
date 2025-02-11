#include "Character/AnimNotify/AttachSheathingSocketNotifyState.h"
#include "Character/Player/PlayerCharacter.h"

void UAttachSheathingSocketNotifyState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (const TObjectPtr<APlayerCharacter> OwnerPlayer = Cast<APlayerCharacter>(MeshComp->GetOwner()))
	{
		OwnerPlayer->GetWeaponController()->SheathingWeapon();
	}
}
