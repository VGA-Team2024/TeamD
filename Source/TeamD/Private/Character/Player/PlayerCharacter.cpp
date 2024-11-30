#include "Character/Player/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GAS/Monster/MonsterAttributeSet.h"
#include "GAS/Player/PlayerAttributeSet.h"

APlayerCharacter::APlayerCharacter()
{
	// コンポーネントの初期化
	CharacterAttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("PlayerAttributeSet"));
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerMesh = GetMesh();
	SetupInput();
	ApplyWeapon();

	// widgetの表示
	if (PlayerCommonWidgetClass)
	{
		if (const TObjectPtr<UUserWidget> WidgetInstance = CreateWidget<UUserWidget>(GetWorld(), PlayerCommonWidgetClass))
		{
			WidgetInstance->AddToViewport();
		}
	}
}

UPlayerAttributeSet* APlayerCharacter::GetPlayerAttributeSet()
{
	return Cast<UPlayerAttributeSet>(CharacterAttributeSet);
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
			EnhancedInputComponent->BindAction(NormalAttackInput, ETriggerEvent::Started, this, &APlayerCharacter::NormalAttack);
			EnhancedInputComponent->BindAction(DodgeInput, ETriggerEvent::Started, this, &APlayerCharacter::PressedDodge);
			EnhancedInputComponent->BindAction(DodgeInput, ETriggerEvent::Completed, this, &APlayerCharacter::ReleasedDodge);
			EnhancedInputComponent->BindAction(DashInput, ETriggerEvent::Started, this, &APlayerCharacter::PressedDash);
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
	// 抜刀状態かの確認
	if (IsDrawing) // 抜刀中
	{
		// SaveInput状態化の判定
		if (CustomAbilitySystemComponent->HasMatchingGameplayTag(SaveInputStateTag))
		{
			// SaveInputを有効にする
			CustomAbilitySystemComponent->AddLooseGameplayTag(NormalAttackTag);
		}
		else
		{
			// 攻撃アビリティの再生
			CustomAbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(NormalAttackTag), true);
		}
	}
	else
	{
		// 抜刀アビリティの再生
		CustomAbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(DrawingSwordTag), true);
		
		IsDrawing = true;
	}
}

void APlayerCharacter::PressedDodge()
{
	Jump();
}

void APlayerCharacter::ReleasedDodge()
{
	StopJumping();
}

void APlayerCharacter::PressedDash()
{
	if (IsDrawing)
	{
		// 納刀アビリティの再生
		CustomAbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(SheathingOfSwordTag), true);
		IsDrawing = false;
		
		return;
	}

	// todo ダッシュってあるのか？
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
	WeaponActor->AttachSheathingSocket(PlayerMesh);

	// 武器のAbilityをPlayerに持たせる
	for (auto Ability : WeaponActor->AttackAbilities)
	{
		CustomAbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability.GetDefaultObject(), 0, -1));
	}

	// OnHit
	WeaponActor->OnHitAttack.AddDynamic(this, &APlayerCharacter::DealDamage);
	WeaponActor->OnHitAttack.AddDynamic(this, &APlayerCharacter::AnimHitStop);
}

void APlayerCharacter::DealDamage(FHitResult HitResult)
{
	if (const ACharacterBase* TargetCharacter = Cast<ACharacterBase>(HitResult.GetActor()))
	{
		// Spec作成
		FGameplayEffectContextHandle ContextHandle = CustomAbilitySystemComponent->MakeEffectContext();
		// HitResultにダメージを与えたActorを登録する
		ContextHandle.AddHitResult(HitResult);
		const FGameplayEffectSpecHandle SpecHandle = CustomAbilitySystemComponent->MakeOutgoingSpec(DealDamageEffectClass, 0, ContextHandle);

		if (SpecHandle.IsValid())
		{
			// Effectの適用
			CustomAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetCharacter->GetAbilitySystemComponent());
		}
	}
}

void APlayerCharacter::OnDealtDamage(float Damage, FVector HitPoint)
{
	if (DamageUIClass)
	{
		if (const TObjectPtr<UDamageDisplayWidget> DamageUIInstance = CreateWidget<UDamageDisplayWidget>(GetWorld(), DamageUIClass))
		{
			DamageUIInstance->AddToViewport();

			if (const TObjectPtr<APlayerController> PlayerController = Cast<APlayerController>(GetController()))
			{
				DamageUIInstance->InitDamageDisplay(Damage, HitPoint, PlayerController);
			}
		}
	}
}

void APlayerCharacter::AnimHitStop(FHitResult HitResult)
{
	TObjectPtr<UAnimInstance> AnimInstance;
	TObjectPtr<UAnimMontage> CurrentMontage;

	// nullチェックと代入
	if (!PlayerMesh || !((AnimInstance = PlayerMesh->GetAnimInstance())) || !((CurrentMontage = AnimInstance->GetCurrentActiveMontage()))) return;

	// 再生を一時停止
	AnimInstance->Montage_SetPlayRate(CurrentMontage, StopSpeed);

	// タイマーセット ヒットストップの時間はワールド時間
	FTimerHandle TimerHandle;
	// 明示的にFTimerDelegateをバインドすることでエラー起きないみたい
	FTimerDelegate TimerDelegate;

	TimerDelegate.BindLambda([this, AnimInstance, CurrentMontage]()
	{
		if (AnimInstance)
		{
			// 元の再生速度に戻す
			AnimInstance->Montage_SetPlayRate(CurrentMontage, 1.f);
		}
	});
	
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, HitStopDuration, false); // todo ストップ時間の参照
}
