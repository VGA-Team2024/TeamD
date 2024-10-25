#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemComponent.h"
#include "WeaponBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitDelegate, AActor*, Target);

// プレイヤーの武器の基底
UCLASS()
class TEAMD_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:
	AWeaponBase();

protected:
	virtual void BeginPlay() override;

	// 武器の攻撃当たり判定
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "Components")
	TObjectPtr<UCapsuleComponent> WeaponAttackCollision;

	// 武器のStaticMesh
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* WeaponMesh;

	// 攻撃当たり判定の開始コールバック
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	// 攻撃判定開始
	void BeginWeaponAttack();

	// 攻撃判定終了
	void EndWeaponAttack();

	FOnHitDelegate OnHitAttack;
	
	// 武器の攻撃Ability
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UGameplayAbility>> AttackAbilities;

//------------------------アタッチ------------------------

	// 納刀のソケットにアタッチする
	void AttachSheathingSocket(USkeletalMeshComponent* AttachMesh);
	
	// 抜刀のソケットにアタッチする
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attach)
	TObjectPtr<USceneComponent> DrawingAttachPivot;
};
