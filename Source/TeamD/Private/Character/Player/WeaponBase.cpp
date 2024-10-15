#include "Character/Player/WeaponBase.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// root
	USceneComponent* DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	// 武器の当たり判定
	WeaponAttackCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	WeaponAttackCollision->SetupAttachment(RootComponent);
	// 武器のメッシュ
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	WeaponMesh->SetupAttachment(WeaponAttackCollision);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	if (WeaponAttackCollision)
	{
		WeaponAttackCollision->IgnoreActorWhenMoving(this, true);
		WeaponAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnBeginOverlap);
		WeaponAttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("null WeaponAttackCollision"));
	}
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

