#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "GAS/Monster/MonsterAttributeSet.h"
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

	UPROPERTY(BlueprintReadOnly, Category = GAS)
	TObjectPtr<UMonsterAttributeSet> MonsterAttributeSet;
};
