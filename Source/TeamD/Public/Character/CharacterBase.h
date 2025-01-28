#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GAS/CustomAbilitySystemComponent.h"
#include "CharacterBase.generated.h"

UCLASS()
class TEAMD_API ACharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ACharacterBase();
	
	virtual void PostInitializeComponents() override;
	
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

//------------------GAS------------------
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GAS, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCustomAbilitySystemComponent> CustomAbilitySystemComponent = nullptr;

public:
	virtual UCustomAbilitySystemComponent* GetAbilitySystemComponent() const override { return CustomAbilitySystemComponent; }

	// 現在ActiveなAbilityを取得する
	UFUNCTION(BlueprintCallable, Category = GAS)
	void GetActiveAbilities(TArray<UGameplayAbility*>& ActiveAbilities);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = GAS)
	TObjectPtr<UAttributeSet> CharacterAttributeSet;

protected:
	// このCharacterに初期セットするAbility
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GAS)
	TArray<TSubclassOf<UGameplayAbility>> InitialAbilities;

	// AttributeSetの値をステータスに初期化する
	void InitAttributeSetParam();
	
	// 初期パラメータのEffect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GAS) // todo:カテゴリーはこれでいいのか
	TSubclassOf<UGameplayEffect> InitStatusEffectClass;
};
