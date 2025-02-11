#include "Character/AnimNotify/AttachDrawingSocketAnimNotify.h"
#include "Character/Player/PlayerCharacter.h"

void UAttachDrawingSocketAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (const TObjectPtr<APlayerCharacter> OwnerPlayer = Cast<APlayerCharacter>(MeshComp->GetOwner()))
	{
		OwnerPlayer->GetWeaponController()->DrawingWeapon();
	}
}
