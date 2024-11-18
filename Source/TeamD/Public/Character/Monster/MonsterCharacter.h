#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "MonsterCharacter.generated.h"

// モンスターの部位情報
USTRUCT(BlueprintType)
struct FMonsterBodyPart
{
	GENERATED_BODY()

	// 名前
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName PartName;

	// 肉質
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MeatQuality = 100.f;

	// 部位のコリジョンがついているボーンの名前 改行区切り
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (MultiLine = true))
	FText BoneNames;
};

/**
 * 
 */
UCLASS()
class TEAMD_API AMonsterCharacter : public ACharacterBase
{
	GENERATED_BODY()

public:
	AMonsterCharacter();

	virtual void BeginPlay() override;

	//--------------------攻撃を与える--------------------
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attack)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	TArray<FName> EnableShapesBoneName;
	
	//--------------------攻撃を受ける--------------------
	
	// モンスターの部位情報
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Status)
	TArray<FMonsterBodyPart> BodyParts;

	// ボーンの名前から部位を返す
	FMonsterBodyPart* ConvertBoneNameToPart(const FName& TargetBoneName);
};