#include "Character/Monster/MonsterCharacter.h"
#include "GAS/Monster/MonsterAttributeSet.h"

AMonsterCharacter::AMonsterCharacter()
{
	CharacterAttributeSet = CreateDefaultSubobject<UMonsterAttributeSet>(TEXT("MonsterAttributeSet"));
}

void AMonsterCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	
}
