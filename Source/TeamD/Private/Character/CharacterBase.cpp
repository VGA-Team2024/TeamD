#include "Character/CharacterBase.h"

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 初期化
	CustomAbilitySystemComponent = CreateDefaultSubobject<UCustomAbilitySystemComponent>(TEXT("CustomAbilitySystemComponent"));
}

void ACharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	// ASCの初期化が必要らしい
	if (CustomAbilitySystemComponent)
	{
		CustomAbilitySystemComponent->InitAbilityActorInfo(this, this);
		UE_LOG(LogTemp, Log, TEXT("init custom asc"));
	}
	else UE_LOG(LogTemp, Error, TEXT("APlayerCharacter::PostInitializeComponents : null CustomAbilitySystemComponent"));
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (CustomAbilitySystemComponent)
	{
		CustomAbilitySystemComponent->InitAbilityActorInfo(this, this);
		UE_LOG(LogTemp, Log, TEXT("init custom asc"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("APlayerCharacter::BeginPlay : null CustomAbilitySystemComponent"));
		return;
	}

	// Abilityを与える
	for (auto Ability : InitialAbilities)
	{
		CustomAbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability.GetDefaultObject(), 0, -1, this));
	}

	InitAttributeSetParam();
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

void ACharacterBase::InitAttributeSetParam()
{
	if (InitStatusEffectClass)
	{
		// Spec作成
		FGameplayEffectContextHandle ContextHandle = CustomAbilitySystemComponent->MakeEffectContext();
		const FGameplayEffectSpecHandle SpecHandle = CustomAbilitySystemComponent->MakeOutgoingSpec(InitStatusEffectClass, 0, ContextHandle);

		if (SpecHandle.IsValid())
		{
			// Effectの適用
			CustomAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), CustomAbilitySystemComponent);
		}
	}
}

