#include "Character/Player/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

APlayerCharacter::APlayerCharacter()
{
	// コンポーネントの初期化
	PlayerAttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("PlayerAttributeSet"));
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerMesh = FindComponentByClass<USkeletalMeshComponent>();
	SetupInput();
	ApplyWeapon();
}

void APlayerCharacter::SetupInput()
{
	// PlayerControllerの取得
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	// 入力を有効にする
	EnableInput(PlayerController);

	if (InputComponent)
	{
		// アクションをバインドする
		if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent)) {

			// Bind Action
			EnhancedInputComponent->BindAction(MoveInput, ETriggerEvent::Triggered, this, &APlayerCharacter::MovePlayer);
			EnhancedInputComponent->BindAction(LookInput, ETriggerEvent::Triggered, this, &APlayerCharacter::RotateControllerInput);
			EnhancedInputComponent->BindAction(NormalAttackInput, ETriggerEvent::Triggered, this, &APlayerCharacter::NormalAttack);
			EnhancedInputComponent->BindAction(DodgeInput, ETriggerEvent::Triggered, this, &APlayerCharacter::PressedDodge);
			EnhancedInputComponent->BindAction(DodgeInput, ETriggerEvent::Completed, this, &APlayerCharacter::ReleasedDodge);
		}

		// Input Mapping Contextを登録する
		if (PlayerController)
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void APlayerCharacter::MovePlayer(const FInputActionValue& Value)
{
	// Vector2dにする
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		// 正面を取得する
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		// 右
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APlayerCharacter::RotateControllerInput(const FInputActionValue& Value)
{
	// Vector2dにする
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller)
	{
		// コントローラーに加算する
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayerCharacter::NormalAttack()
{
	AbilitySystemComponent->TryActivateAbilitiesByTag(NormalAttackTag, true);
}

void APlayerCharacter::PressedDodge()
{
	Jump();
}

void APlayerCharacter::ReleasedDodge()
{
	StopJumping();
}

void APlayerCharacter::ApplyWeapon()
{
	// スポーンのパラメーター
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;  // スポーンしたアクターのオーナーを設定
	SpawnParams.Instigator = GetInstigator();  // スポーンしたアクターのインスティゲーターを設定

	// アクターを生成
	WeaponActor = GetWorld()->SpawnActor<AWeaponBase>(PlayerEquipment.Weapon, GetActorLocation(), GetActorRotation(), SpawnParams);

	// Meshにアタッチ　あってるか分からん
	WeaponActor->AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponActor->AttachSocketName);

	// 武器のAbilityをPlayerに持たせる
	for (auto Ability : WeaponActor->AttackAbilities)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability.GetDefaultObject(), 0, -1));
	}

	// OnHit
	WeaponActor->OnHitAttack.AddDynamic(this, &APlayerCharacter::DealDamage);
}

void APlayerCharacter::DealDamage(AActor* Target)
{
	if (const ACharacterBase* TargetCharacter = Cast<ACharacterBase>(Target))
	{
		// Spec作成
		const FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DealDamageEffectClass, 0, EffectContext);

		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetCharacter->GetAbilitySystemComponent());
		}
	}
}
