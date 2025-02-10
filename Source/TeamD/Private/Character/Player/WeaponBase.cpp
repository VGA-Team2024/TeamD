#include "Character/Player/WeaponBase.h"
#include "Atom/AtomComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Framework/CustomFramework.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// root
	const TObjectPtr<USceneComponent> DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;
	
	// 武器のメッシュ
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	WeaponMesh->SetupAttachment(DefaultSceneRoot);
	// 武器の当たり判定
	WeaponAttackCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	WeaponAttackCollision->SetupAttachment(WeaponMesh);
	// SceneComponent
	SheathingAttachPivot = CreateDefaultSubobject<USceneComponent>(TEXT("SheathingPivotSceneComponent"));
	SheathingAttachPivot->SetupAttachment(WeaponMesh);
	DrawingAttachPivot = CreateDefaultSubobject<USceneComponent>(TEXT("DrawingPivotSceneComponent"));
	DrawingAttachPivot->SetupAttachment(WeaponMesh);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	if (WeaponAttackCollision)
	{
		WeaponAttackCollision->IgnoreActorWhenMoving(this, true);
		WeaponAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponAttackCollision->IgnoreActorWhenMoving(GetOwner(), true);
	}
	else UE_LOG(LogTemp, Warning, TEXT("null WeaponAttackCollision"));
}

void AWeaponBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckAttackCollision();
}

void AWeaponBase::BeginWeaponAttack()
{
	if (WeaponAttackCollision) bCanHit = true;
}

void AWeaponBase::EndWeaponAttack()
{
	if (WeaponAttackCollision) bCanHit = false;
}

void AWeaponBase::CheckAttackCollision()
{
	if (!bCanHit)
	{
		LastCollisionPosition = WeaponAttackCollision->GetComponentLocation();
		return;
	}

	FHitResult HitResult;
	const FVector EndLocation = WeaponAttackCollision->GetComponentLocation();

	// Sweepで当たり判定をとる
	if (GetWorld()->SweepSingleByChannel(HitResult, LastCollisionPosition, EndLocation, FQuat(WeaponAttackCollision->GetComponentRotation()),
		CollisionChannel, WeaponAttackCollision->GetCollisionShape()))
	{
		// 当たった処理を呼び出す
		OnAttackHit(HitResult);
	}
	
	LastCollisionPosition = EndLocation;
}

void AWeaponBase::OnAttackHit(const FHitResult& HitResult)
{
	// ヒット回数
	bCanHit = false;
	
	// ヒット音の再生
	const TObjectPtr<UAtomComponent> AtomComponent = NewObject<UAtomComponent>(this);

	if (AtomComponent && HitSound)
	{
		AtomComponent->SetSound(HitSound);
		AtomComponent->Play();
	}

	// ヒットエフェクトの再生
	SpawnNiagaraEffect(HitEffect, HitResult.ImpactPoint, FRotator::ZeroRotator);
	
	OnHitAttack.Broadcast(HitResult);
}

void AWeaponBase::SpawnNiagaraEffect(const TObjectPtr<UNiagaraSystem>& NiagaraEffect, const FVector& Location, const FRotator& Rotation) const
{
	if (!NiagaraEffect)
	{
		LOG_INFO(Warning, TEXT("SpawnNiagaraEffect is nullptr"));
		return;
	}
	
	if (const TObjectPtr<UWorld> World = GetWorld())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, NiagaraEffect, Location, Rotation);
	}
}
