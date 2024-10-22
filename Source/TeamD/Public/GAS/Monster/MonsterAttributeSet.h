#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MonsterAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class TEAMD_API UMonsterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UMonsterAttributeSet();
	
	// 動的に変わるパラメータのClampとイベント発火をする
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// 現在体力　MaxHealthが上限
	UPROPERTY(BlueprintReadOnly, Category = Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UMonsterAttributeSet, Health)

	// 現在体力　MaxHealthが上限
	UPROPERTY(BlueprintReadOnly, Category = Health)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UMonsterAttributeSet, MaxHealth)
};
