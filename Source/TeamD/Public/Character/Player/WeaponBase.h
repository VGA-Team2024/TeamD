#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemComponent.h"
#include "Atom/AtomSoundBase.h"
#include "WeaponBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitDelegate, FHitResult, HitResult);

// 武器の性能パラメータ
USTRUCT(BlueprintType)
struct FWeaponStatus
{
	GENERATED_BODY()

	// 攻撃力
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int AttackPower;

	FWeaponStatus()
		: AttackPower(10)
	{
	}
};

/**
 * プレイヤーの武器の基底
 */
UCLASS()
class TEAMD_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:
	AWeaponBase();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	// 武器の攻撃当たり判定
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "Components")
	TObjectPtr<UCapsuleComponent> WeaponAttackCollision;

	// 武器のStaticMesh
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

public:
	// 武器の性能パラメータ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FWeaponStatus WeaponStatus;

	//------------------------攻撃判定------------------------

	// 攻撃判定開始
	void BeginWeaponAttack();

	// 攻撃判定終了
	void EndWeaponAttack();

	// 当たり判定チェック
	void CheckAttackCollision();

	// 攻撃が当たったとき
	void OnAttackHit(const FHitResult& HitResult);

	// 攻撃ごとのヒットを使ったかどうか todo モンハンは基本的にヒット回数１回だけど...
	bool bCanHit;

	FVector LastCollisionPosition;

	FOnHitDelegate OnHitAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TEnumAsByte<ECollisionChannel> CollisionChannel;
	
	// 武器の攻撃Ability
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UGameplayAbility>> AttackAbilities;

	//------------------------アタッチ------------------------
	
	// 納刀状態にアタッチするソケット名
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attach)
	FName SheathingAttachSocketName;

	// 納刀状態ソケットと重なるやつ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attach)
	TObjectPtr<USceneComponent> SheathingAttachPivot;

	// 抜刀状態にアタッチするソケット名
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attach)
	FName DrawingAttachSocketName;

	// 抜刀状態ソケットと重なるやつ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attach, meta = (DisplayPriority = -1))
	TObjectPtr<USceneComponent> DrawingAttachPivot;

	//------------------------音------------------------

	// 攻撃がヒットしたときの音 全武器共通でもここにあったほうが
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	TObjectPtr<UAtomSoundBase> HitSound;
};
