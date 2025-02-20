#include "InGame/AnimNotify/Monster/RotateToTargetNotifyState.h"
#include "Character/Monster/MonsterCharacter.h"
#include "Framework/CustomFramework.h"

void URotateToTargetNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                            float FrameDeltaTime)
{
	const TObjectPtr<AMonsterCharacter> OwnerMonster = Cast<AMonsterCharacter>(MeshComp->GetOwner());

	if (!OwnerMonster)
	{
		return;
	}

	RotateToTarget(OwnerMonster, OwnerMonster->GetAIController()->GetTargetActor(), FrameDeltaTime);
}

void URotateToTargetNotifyState::RotateToTarget(const TObjectPtr<AActor>& OwnerActor, const TObjectPtr<AActor>& TargetActor,
	float DeltaTime)
{
	if (!OwnerActor || !TargetActor)
	{
		LOG_INFO(Error, "Actor is null");
		return;
	}

	FVector OwnerForward = OwnerActor->GetActorForwardVector();
	OwnerForward.Z = 0;
	OwnerForward.Normalize();
	FVector DirectionToTarget = TargetActor->GetActorLocation() - OwnerActor->GetActorLocation();
	DirectionToTarget.Z = 0;
	DirectionToTarget.Normalize();

	// なす角
	const float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(OwnerForward, DirectionToTarget)));

	if (Angle <= RotateSpeed * DeltaTime)
	{
		// そのフレームの移動量で超える場合、Targetに向く
		OwnerActor->SetActorRotation(DirectionToTarget.Rotation());
	}
	else
	{
		// RotateSpeed分だけ向く
		OwnerActor->SetActorRotation(FVector::SlerpNormals(OwnerForward, DirectionToTarget, RotateSpeed * DeltaTime / Angle).Rotation());
	}
}
