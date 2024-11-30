#include "Character/AnimNotify/MonsterAttackNotifyState.h"
#include "Character/Monster/MonsterCharacter.h"

void UMonsterAttackNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                            float TotalDuration)
{
	if (const TObjectPtr<AMonsterCharacter> OwnerMonster = Cast<AMonsterCharacter>(MeshComp->GetOwner()))
	{
		OwnerMonster->EnableShapesBoneName = EnableShapesBoneName;
	}
}

void UMonsterAttackNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (const TObjectPtr<AMonsterCharacter> OwnerMonster = Cast<AMonsterCharacter>(MeshComp->GetOwner()))
	{
		OwnerMonster->EnableShapesBoneName.Empty();
	}
}
