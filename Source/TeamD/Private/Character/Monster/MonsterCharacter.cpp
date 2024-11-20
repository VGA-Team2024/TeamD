#include "Character/Monster/MonsterCharacter.h"
#include "Character/Player/PlayerCharacter.h"
#include "GAS/Monster/MonsterAttributeSet.h"

AMonsterCharacter::AMonsterCharacter()
{
	CharacterAttributeSet = CreateDefaultSubobject<UMonsterAttributeSet>(TEXT("MonsterAttributeSet"));
}

void AMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetMesh())
	{
		GetMesh()->OnComponentHit.AddDynamic(this, &AMonsterCharacter::OnHit);
	}
}

void AMonsterCharacter::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// 攻撃中のShapeかチェック
	if (EnableShapesBoneName.Contains(Hit.MyBoneName))
	{
		// 当たった対象がPlayerかチェック
		if (const TObjectPtr<APlayerCharacter> TargetPlayer = Cast<APlayerCharacter>(OtherActor))
		{
			// Spec作成
			FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
			// HitResultにダメージを与えたActorを登録する
			ContextHandle.AddHitResult(Hit);
			const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DamageEffectClass, 0, ContextHandle);

			if (SpecHandle.IsValid())
			{
				// Effectの適用
				AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetPlayer->GetAbilitySystemComponent());
			}

			TargetPlayer->GetMesh()->GetAnimInstance()->Montage_Play(PlayerHitMontage);
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
