#include "Character/AnimNotify/AttachDrawingSocketAnimNotify.h"
#include "Character/Player/PlayerCharacter.h"

void UAttachDrawingSocketAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (const TObjectPtr<APlayerCharacter> Player = Cast<APlayerCharacter>(MeshComp->GetOwner()))
	{
		Player->WeaponActor->AttachDrawingSocket(MeshComp);
	}
}
