#pragma once

#include "CoreMinimal.h"
#include "LevelSequence.h"
#include "MonsterAIController.h"
#include "Character/CharacterBase.h"
#include "GAS/Monster/MonsterAttributeSet.h"
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

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UMonsterAttributeSet* GetMonsterAttributeSet() const { return Cast<UMonsterAttributeSet>(CharacterAttributeSet); }

	// MonsterのAIControllerを取得する
	TObjectPtr<AMonsterAIController> GetAIController() const { return Cast<AMonsterAIController>(GetController()); }

	//--------------------攻撃を与える--------------------

	// Meshからのコールバック
	UFUNCTION()
	void OnHitMesh(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// AttackCollisionからのコールバック
	UFUNCTION()
	void OnBeginOverlapAttack(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);

	// 特定のボーンにアタッチされているAttackCollisionのResponseを変更する
	void SetAttackCollisionResponse(FName BoneName, ECollisionResponse NewResponse);
	
	// ダメージを呼び出す
	UFUNCTION(BlueprintCallable)
	void DealDamage(AActor* TargetActor);

	TArray<TObjectPtr<UShapeComponent>> AttackCollisions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attack)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	TArray<FName> EnableShapesBoneName;
	
	//--------------------攻撃を受ける--------------------
	
	// モンスターの部位情報
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Status)
	TArray<FMonsterBodyPart> BodyParts;

	// ボーンの名前から部位を返す
	FMonsterBodyPart* ConvertBoneNameToPart(const FName& TargetBoneName);

protected:
	// ダメージ受けたときのコールバック
	UFUNCTION()
	void OnReceiveDamage(float Health);

	// 死んだときのシーケンス
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, category = Damage)
	TSoftObjectPtr<ULevelSequence> DeadSequenceAsset;

	// 死んだとき
	void OnDead();
};