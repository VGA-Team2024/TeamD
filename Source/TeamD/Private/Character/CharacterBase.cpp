#include "Character/CharacterBase.h"

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 初期化
	CustomAbilitySystemComponent = nullptr;
	CustomAbilitySystemComponent = CreateDefaultSubobject<UCustomAbilitySystemComponent>(TEXT("CustomAbilitySystemComponent"));

	if (!CustomAbilitySystemComponent) UE_LOG(LogTemp, Warning, TEXT("ASCが無い"));
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// Abilityを与える
	for (auto Ability : InitialAbilities)
	{
		CustomAbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability.GetDefaultObject(), 0, -1, this));
	}
}

void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACharacterBase::GetActiveAbilities(TArray<UGameplayAbility*>& ActiveAbilities)
{
	for (auto AbilitySpec : CustomAbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.IsActive())
		{
			for (UGameplayAbility* ActiveAbility : AbilitySpec.GetAbilityInstances())
			{
				ActiveAbilities.Add(ActiveAbility);
			}
		}
	}
}

