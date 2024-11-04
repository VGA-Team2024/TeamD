#include "Character/Player/WeaponBase.h"

#include "Character/Monster/MonsterCharacter.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// root
	USceneComponent* DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	// 武器のメッシュ
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	WeaponMesh->SetupAttachment(RootComponent);
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

	if (WeaponAttackCollision.IsValid())
	{
		WeaponAttackCollision->IgnoreActorWhenMoving(this, true);
		WeaponAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnBeginOverlap);
		WeaponAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponAttackCollision->IgnoreActorWhenMoving(GetOwner(), true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("null WeaponAttackCollision"));
	}

	
}

void AWeaponBase::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// todo コリジョンプリセットで当たらないようにしたい
	if (!Cast<AMonsterCharacter>(OtherActor)) return;
	
	OnHitAttack.Broadcast(OtherActor);
	
	UE_LOG(LogTemp, Log, TEXT("Hit Actor Name : %s"), *OtherActor->GetName());
}

void AWeaponBase::BeginWeaponAttack()
{
	if (WeaponAttackCollision.IsValid())
	{
		WeaponAttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void AWeaponBase::EndWeaponAttack()
{
	if (WeaponAttackCollision.IsValid())
	{
		WeaponAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWeaponBase::AttachSheathingSocket(USkeletalMeshComponent* AttachMesh)
{
	// ソケットの切替
	if (AttachMesh->DoesSocketExist(SheathingAttachSocketName))
	{
		AttachToComponent(AttachMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SheathingAttachSocketName);
		// 位置、回転の補正
		const FRotator RelativeRotate = SheathingAttachPivot->GetRelativeRotation().GetInverse();
		SetActorRelativeRotation(RelativeRotate);
		SetActorRelativeLocation(RelativeRotate.RotateVector(-SheathingAttachPivot->GetRelativeLocation()));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ソケットないよ"));
	}
}

void AWeaponBase::AttachDrawingSocket(USkeletalMeshComponent* AttachMesh)
{
	// ソケットの切替
	if (AttachMesh->DoesSocketExist(DrawingAttachSocketName))
	{
		AttachToComponent(AttachMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, DrawingAttachSocketName);
		// 位置、回転の補正
		const FRotator RelativeRotate = DrawingAttachPivot->GetRelativeRotation().GetInverse();
		SetActorRelativeRotation(RelativeRotate);
		SetActorRelativeLocation(RelativeRotate.RotateVector(-DrawingAttachPivot->GetRelativeLocation()));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ソケットないよ"));
	}
}

