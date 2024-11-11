#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemComponent.h"
#include "WeaponBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitDelegate, AActor*, Target);

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

// プレイヤーの武器の基底
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

	//------------------------攻撃------------------------

	// 攻撃当たり判定の開始コールバック
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 攻撃判定開始
	void BeginWeaponAttack();

	// 攻撃判定終了
	void EndWeaponAttack();

	void CheckAttackCollision();

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

	// 納刀のソケットにアタッチする
	UFUNCTION(BlueprintCallable, Category = Attach)
	void AttachSheathingSocket(USkeletalMeshComponent* AttachMesh);
	
	// 抜刀のソケットにアタッチする
	UFUNCTION(BlueprintCallable, Category = Attach)
	void AttachDrawingSocket(USkeletalMeshComponent* AttachMesh);
	
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

	//------------------------デバッグ------------------------
	
	bool IsDrawing = false;
	
	// TickでPivotの調整を可能にするか
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	bool ApplyPivotOnTick = true;
};
