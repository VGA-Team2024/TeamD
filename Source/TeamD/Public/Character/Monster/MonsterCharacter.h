#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "Components/BoxComponent.h"
#include "MonsterCharacter.generated.h"

/**
 * 
 */
UCLASS()
class TEAMD_API AMonsterCharacter : public ACharacterBase
{
	GENERATED_BODY()

public:
	AMonsterCharacter();

	virtual void OnConstruction(const FTransform& Transform) override;

	// UPROPERTY(BlueprintReadOnly, Category = GAS)
	// TObjectPtr<UMonsterAttributeSet> MonsterAttributeSet;

	//------------------Status------------------

	
};