#include "Character/Player/WeaponBase.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	WeaponAttackCollision->IgnoreActorWhenMoving(this, true);
	WeaponAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnBeginOverlap);
	WeaponAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeaponBase::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Log, TEXT("Hit Actor Name : %s"), *OtherActor->GetName());
}

void AWeaponBase::BeginWeaponAttack()
{
	WeaponAttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AWeaponBase::EndWeaponAttack()
{
	WeaponAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

