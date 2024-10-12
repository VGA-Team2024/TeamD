#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "CharacterBase.generated.h"

UCLASS()
class TEAMD_API ACharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ACharacterBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

//------------------GAS------------------
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Abilities, meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	TObjectPtr<UAttributeSet> AttributeSet;

	// このCharacterに初期セットするAbility
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	TArray<TSubclassOf<UGameplayAbility>> InitialAbilities;
};
