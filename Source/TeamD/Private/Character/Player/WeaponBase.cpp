#include "Character/Player/WeaponBase.h"
#include "Character/Monster/MonsterCharacter.h"

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
		//WeaponAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnBeginOverlap);
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

void AWeaponBase::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bCanHit) return;
	
	OnHitAttack.Broadcast(OtherActor);
	bCanHit = false;
	UE_LOG(LogTemp, Warning, TEXT("Hit Actor : %s, Comp : %s, Bone : %s, Is Sweep : %hd"), *OtherActor->GetName(),
		*OtherComp->GetName(), *SweepResult.BoneName.ToString(), bFromSweep);
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

	if (GetWorld()->SweepSingleByChannel(HitResult, LastCollisionPosition, EndLocation, FQuat(WeaponAttackCollision->GetComponentRotation()),
		CollisionChannel, WeaponAttackCollision->GetCollisionShape()))
	{
		OnHitAttack.Broadcast(HitResult.GetActor());
		bCanHit = false;
		//UE_LOG(LogTemp, Warning, TEXT("Hit Actor : %s, Comp : %s, Bone : %s"), *HitResult.GetActor()->GetName(),
			//*HitResult.GetComponent()->GetName(), *HitResult.BoneName.ToString());

		if (AMonsterCharacter* Monster = Cast<AMonsterCharacter>(HitResult.GetActor()))
		{
			Monster->ConvertBoneNameToPart(HitResult.BoneName);
		}
	}
	
	LastCollisionPosition = EndLocation;
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

