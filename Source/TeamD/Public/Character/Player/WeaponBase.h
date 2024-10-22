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

	// 攻撃判定開始
	UFUNCTION(BlueprintCallable)
	void BeginWeaponAttack();

	// 攻撃判定終了
	UFUNCTION(BlueprintCallable)
	void EndWeaponAttack();

public:
	FOnHitDelegate OnHitAttack;
	
	// 武器の攻撃Ability
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UGameplayAbility>> AttackAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName AttachSocketName;
};
