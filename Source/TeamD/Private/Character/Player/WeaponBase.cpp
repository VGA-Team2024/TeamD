#include "Character/Player/WeaponBase.h"
#include "Atom/AtomComponent.h"

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
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("null WeaponAttackCollision"));
	}

	
}

void AWeaponBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ApplyPivotOnTick)
	{
		if (IsDrawing)
		{
			const FRotator RelativeRotate = DrawingAttachPivot->GetRelativeRotation().GetInverse();
			SetActorRelativeRotation(RelativeRotate);
			SetActorRelativeLocation(RelativeRotate.RotateVector(-DrawingAttachPivot->GetRelativeLocation()));
		}
		else
		{
			const FRotator RelativeRotate = SheathingAttachPivot->GetRelativeRotation().GetInverse();
			SetActorRelativeRotation(RelativeRotate);
			SetActorRelativeLocation(RelativeRotate.RotateVector(-SheathingAttachPivot->GetRelativeLocation()));
		}
	}

	CheckAttackCollision();
}

void AWeaponBase::SheathingWeapon(const APlayerCharacter* TargetPlayer)
{
	// アタッチして武器の位置を変える
	AttachSheathingSocket(TargetPlayer->GetMesh());

	// todo:不器用の入力を消す
}

void AWeaponBase::DrawingWeapon(const APlayerCharacter* TargetPlayer)
{
	// todo:
}

void AWeaponBase::BeginWeaponAttack()
{
	if (WeaponAttackCollision)
	{
		WeaponAttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		bCanHit = true;
	}
}

void AWeaponBase::EndWeaponAttack()
{
	if (WeaponAttackCollision)
	{
		WeaponAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		bCanHit = false;
	}
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
	OnHitAttack.Broadcast(HitResult);
	// ヒット回数
	bCanHit = false;
	
	// ヒット音の再生
	const TObjectPtr<UAtomComponent> AtomComponent = NewObject<UAtomComponent>(this);

	if (AtomComponent && HitSound)
	{
		AtomComponent->SetSound(HitSound);
		AtomComponent->Play();
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
		IsDrawing = false;
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
		IsDrawing = true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ソケットないよ"));
	}
}

