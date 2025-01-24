#include "Character/Player/WeaponController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/CharacterBase.h"
#include "Kismet/GameplayStatics.h"

UWeaponController::UWeaponController()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWeaponController::BeginPlay()
{
	Super::BeginPlay();

	SetupInput();
	GetCustomASC();
}

void UWeaponController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWeaponController::GetCustomASC()
{
	if (!((CustomAbilitySystemComponent = Cast<ACharacterBase>(GetOwner())->GetAbilitySystemComponent())))
	{
		UE_LOG(LogTemp, Error, TEXT("null CustomAbilitySystemComponent"));
	}
}

void UWeaponController::SetupInput()
{
	// PlayerControllerの取得
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (GetOwner()->InputComponent)
	{
		// アクションをバインドする
		if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(GetOwner()->InputComponent))
		{
			EnhancedInputComponent->BindAction(NormalAttackInput, ETriggerEvent::Started, this, &UWeaponController::NormalAttack);
		}

		// Input Mapping Contextを登録する
		if (PlayerController)
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(WeaponMappingContext, 0);
			}
		}
	}
}

void UWeaponController::NormalAttack()
{
	if (!CustomAbilitySystemComponent) return;

	if (bIsDrawing) CustomAbilitySystemComponent->SaveTagTryActivateAbilities(NormalAttackInputTag);
	else CustomAbilitySystemComponent->SaveTagTryActivateAbilities(DrawingSwordInputTag);
}

