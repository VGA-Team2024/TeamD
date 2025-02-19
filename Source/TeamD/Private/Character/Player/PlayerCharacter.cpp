#include "Character/Player/PlayerCharacter.h"
#include "DefaultLevelSequenceInstanceData.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/Monster/MonsterAttributeSet.h"
#include "GAS/Player/PlayerAttributeSet.h"

APlayerCharacter::APlayerCharacter()
{
	// コンポーネントの初期化
	CharacterAttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("PlayerAttributeSet"));
	WeaponController = CreateDefaultSubobject<UWeaponController>(TEXT("WeaponController"));
	PlayerItemManager = CreateDefaultSubobject<UPlayerItemManager>(TEXT("PlayerItemManager"));
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetupInput();

	// ヒットモーションは別定義だからここで登録する todo:GiveAbilityAndActivateOnceを使ってもいい
	for (auto Ability : DamageMotions)
	{
		CustomAbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability.GetDefaultObject(), 0, -1, this));
	}

	// widgetの表示
	if (PlayerCommonWidgetClass)
	{
		if (const TObjectPtr<UUserWidget> WidgetInstance = CreateWidget<UUserWidget>(GetWorld(), PlayerCommonWidgetClass))
		{
			WidgetInstance->AddToViewport();
		}
	}

	// ダメージコールバックを登録する
	GetPlayerAttributeSet()->OnChangedHealth.AddDynamic(this, &APlayerCharacter::OnReceiveDamage);
	// 移動スピードを取得
	WalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
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
			EnhancedInputComponent->BindAction(DodgeInput, ETriggerEvent::Started, this, &APlayerCharacter::PressedDodge);
			EnhancedInputComponent->BindAction(DashInput, ETriggerEvent::Triggered, this, &APlayerCharacter::DownDash);
			EnhancedInputComponent->BindAction(DashInput, ETriggerEvent::Completed, this, &APlayerCharacter::ReleasedDash);
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
	// 移動時に速度を更新する
	UpdateMovementSpeed();
	
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

void APlayerCharacter::PressedDodge()
{
	CustomAbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(DodgeInputTag), true);
}

void APlayerCharacter::DownDash()
{
	bIsDashing = true;
}

void APlayerCharacter::ReleasedDash()
{
	bIsDashing = false;
}

void APlayerCharacter::UpdateMovementSpeed() const
{
	if (CustomAbilitySystemComponent->HasMatchingGameplayTag(SheathingOrDrawingStateTag))
	{
		// 抜刀納刀中のSpeed
		GetCharacterMovement()->MaxWalkSpeed = SheathingOrDrawingSpeed;
	}
	else if (CustomAbilitySystemComponent->HasMatchingGameplayTag(UsingItemStateTag))
	{
		// Item使用中のSpeed
		GetCharacterMovement()->MaxWalkSpeed = UsingItemSpeed;
	}
	else if (bIsDashing && !WeaponController->GetIsDrawing())
	{
		// Dash入力中 && 納刀中
		GetCharacterMovement()->MaxWalkSpeed = DashSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void APlayerCharacter::OnReceiveDamage(float Damage, const FVector& DamageDirection, const TObjectPtr<UMonsterAttackAbilityBase>& AttackAbility)
{
	// 攻撃の向きに対する自分の向きからリアクションを変える
	// 正面からの攻撃であるか
	bool IsForward = DamageDirection.Dot(GetActorForwardVector()) < 0;
	// todo モーションが無いので向きを正面からで固定してある
	IsForward = true;
	
	// 向きを変える
	SetActorRotation((DamageDirection * (IsForward ? -1.f : 1.f)).Rotation());
	
	// DamageMotionの再生
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(IsForward ? AttackAbility->DamageMotionTagOfFront : AttackAbility->DamageMotionTagOfBehind);
	CustomAbilitySystemComponent->TryActivateAbilitiesByTag(TagContainer);
}

void APlayerCharacter::OnReceiveDamage(float Health)
{
	if (Health <= 0)
	{
		OnDead();
	}
}

void APlayerCharacter::OnDead()
{
	UE_LOG(LogTemp, Log, TEXT("player dead"));
	
	// DeadのSequenceを生成する
	if (!DeadSequence.IsNull())
	{
		ALevelSequenceActor* LevelSequenceActor;
		FMovieSceneSequencePlaybackSettings PlaybackSettings;
		TObjectPtr<ULevelSequencePlayer> SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
			GetWorld(),
			DeadSequence.LoadSynchronous(),
			PlaybackSettings,
			LevelSequenceActor
		);
		// Sequence位置の修正
		LevelSequenceActor->bOverrideInstanceData = true;
		Cast<UDefaultLevelSequenceInstanceData>(LevelSequenceActor->DefaultInstanceData)->TransformOrigin = GetMesh()->GetComponentTransform();
		// 再生
		SequencePlayer->Play();

		// 死亡モーションAbilityの再生
		CustomAbilitySystemComponent->GiveAbilityAndActivateOnce(PlayerDeadAbility);
	}
}
