// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PlayerAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * PlayerのAttributeSet
 */
UCLASS()
class TEAMD_API UPlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPlayerAttributeSet();

	// 動的に変わるパラメータのClampとイベント発火をする
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// 現在体力　MaxHealthが上限
	UPROPERTY(BlueprintReadOnly, Category = Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Health)

	// 最大体力
	UPROPERTY(BlueprintReadOnly, Category = Health)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, MaxHealth)

	// 現在スタミナ　MaxStaminaが上限
	UPROPERTY(BlueprintReadOnly, Category = Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Stamina)

	// 最大スタミナ
	UPROPERTY(BlueprintReadOnly, Category = Stamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, MaxStamina)

	// 攻撃力
	UPROPERTY(BlueprintReadOnly, Category = Damage)
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, AttackPower)

	// 防御力
	UPROPERTY(BlueprintReadOnly, Category = Damage)
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Defense)
};
