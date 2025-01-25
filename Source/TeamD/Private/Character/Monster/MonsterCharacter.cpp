#include "Character/Monster/MonsterCharacter.h"
#include "DefaultLevelSequenceInstanceData.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Character/Player/PlayerCharacter.h"

AMonsterCharacter::AMonsterCharacter()
{
	CharacterAttributeSet = CreateDefaultSubobject<UMonsterAttributeSet>(TEXT("MonsterAttributeSet"));
}

void AMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 攻撃の当たり判定のShapeCompを登録する
	const TArray<TObjectPtr<USceneComponent>>& ChildrenComp = GetMesh()->GetAttachChildren();
	
	for (TObjectPtr<USceneComponent> Child : ChildrenComp)
	{
		TObjectPtr<UShapeComponent> ShapeComp;
		
		if (Child && ((ShapeComp = Cast<UShapeComponent>(Child))))
		{
			AttackCollisions.Add(ShapeComp);
			ShapeComp->OnComponentBeginOverlap.AddDynamic(this, &AMonsterCharacter::OnBeginOverlapAttack);
		}
	}

	// 被ダメージのイベントを登録
	GetMonsterAttributeSet()->OnChangedHealth.AddDynamic(this, &AMonsterCharacter::OnReceiveDamage);
}

void AMonsterCharacter::OnHitMesh(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// 攻撃中のShapeかチェック
	if (EnableShapesBoneName.Contains(Hit.MyBoneName))
	{
		DealDamage(OtherActor);

		// 当たった対象がPlayerかチェック
		// if (const TObjectPtr<APlayerCharacter> TargetPlayer = Cast<APlayerCharacter>(OtherActor))
		// {
		// 	// Spec作成
		// 	FGameplayEffectContextHandle ContextHandle = CustomAbilitySystemComponent->MakeEffectContext();
		// 	// HitResultにダメージを与えたActorを登録する
		// 	const FHitResult HitResult = FHitResult(OtherActor, nullptr, FVector(), FVector());
		// 	ContextHandle.AddHitResult(HitResult);
		// 	const FGameplayEffectSpecHandle SpecHandle = CustomAbilitySystemComponent->MakeOutgoingSpec(DamageEffectClass, 0, ContextHandle);
		//
		// 	if (SpecHandle.IsValid())
		// 	{
		// 		// Effectを適用
		// 		CustomAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetPlayer->GetAbilitySystemComponent());
		// 	}
		// }
	}
}

void AMonsterCharacter::OnBeginOverlapAttack(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	DealDamage(OtherActor);
}

void AMonsterCharacter::SetAttackCollisionResponse(FName BoneName, ECollisionResponse NewResponse)
{
	for (const auto AttackCollision : AttackCollisions)
	{
		if (AttackCollision->GetAttachSocketName() == BoneName)
		{
			AttackCollision->SetCollisionResponseToChannel(ECC_Pawn, NewResponse);
		}
	}
}

void AMonsterCharacter::DealDamage(AActor* TargetActor)
{
	// 当たった対象がPlayerかチェック
	if (const TObjectPtr<APlayerCharacter> TargetPlayer = Cast<APlayerCharacter>(TargetActor))
	{
		// Spec作成
		FGameplayEffectContextHandle ContextHandle = CustomAbilitySystemComponent->MakeEffectContext();
		// HitResultにダメージを与えたActorを登録する
		const FHitResult HitResult = FHitResult(TargetActor, nullptr, FVector(), FVector());
		ContextHandle.AddHitResult(HitResult);
		const FGameplayEffectSpecHandle SpecHandle = CustomAbilitySystemComponent->MakeOutgoingSpec(DamageEffectClass, 0, ContextHandle);

		if (SpecHandle.IsValid())
		{
			// Effectを適用
			CustomAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetPlayer->GetAbilitySystemComponent());
		}
	}
}

FMonsterBodyPart* AMonsterCharacter::ConvertBoneNameToPart(const FName& TargetBoneName)
{
	for (int i = 0; i < BodyParts.Num(); i++)
	{
		TArray<FString> BoneNameArray;
		BodyParts[i].BoneNames.ToString().ParseIntoArrayLines(BoneNameArray);

		// その部位のボーン名をループ
		for (const FString BoneName : BoneNameArray)
		{
			if (BoneName == TargetBoneName)
			{
				// 同じなら出力終了
				UE_LOG(LogTemp, Log, TEXT("部位名 : %s"), *BodyParts[i].PartName.ToString());
				return &BodyParts[i];
			}
		}
	}

	// どこにも当てはまらなかったら最初を返す
	UE_LOG(LogTemp, Log, TEXT("設定されてないボーン名が入力されましたので先頭の要素 : %s, ボーン名 : %s"), *BodyParts[0].PartName.ToString(), *TargetBoneName.ToString());
	return &BodyParts[0];
}

void AMonsterCharacter::OnReceiveDamage(float Health)
{
	if (Health <= 0)
	{
		OnDead();
	}
}

void AMonsterCharacter::OnDead()
{
	UE_LOG(LogTemp, Log, TEXT("monster dead"));

	// DeadのSequenceを生成する
	if (!DeadSequenceAsset.IsNull())
	{
		ALevelSequenceActor* LevelSequenceActor;
		FMovieSceneSequencePlaybackSettings PlaybackSettings;
        TObjectPtr<ULevelSequencePlayer> SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
			GetWorld(),
			DeadSequenceAsset.LoadSynchronous(),
			PlaybackSettings,
			LevelSequenceActor
        );
		// Sequence位置の修正
		LevelSequenceActor->bOverrideInstanceData = true;
		Cast<UDefaultLevelSequenceInstanceData>(LevelSequenceActor->DefaultInstanceData)->TransformOrigin = GetMesh()->GetComponentTransform();
		// 再生
		SequencePlayer->Play();
		
		// 自身は削除
		Destroy();
	}
	else UE_LOG(LogTemp, Error, TEXT("not find DeadSequence"));
}
