#include "Character/AnimNotify/MonsterAttackNotifyState.h"
#include "Character/Monster/MonsterCharacter.h"

void UMonsterAttackNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                            float TotalDuration)
{
	if (const TObjectPtr<AMonsterCharacter> OwnerMonster = Cast<AMonsterCharacter>(MeshComp->GetOwner()))
	{
		OwnerMonster->EnableShapesBoneName = EnableShapesBoneName;

		for (const auto EnableBoneName : EnableShapesBoneName)
		{
			OwnerMonster->SetAttackCollisionResponse(EnableBoneName, ECR_Overlap);
		}
	}
}

void UMonsterAttackNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (const TObjectPtr<AMonsterCharacter> OwnerMonster = Cast<AMonsterCharacter>(MeshComp->GetOwner()))
	{
		OwnerMonster->EnableShapesBoneName.Empty();

		for (const auto EnableBoneName : EnableShapesBoneName)
		{
			OwnerMonster->SetAttackCollisionResponse(EnableBoneName, ECR_Block);
		}
	}
}
