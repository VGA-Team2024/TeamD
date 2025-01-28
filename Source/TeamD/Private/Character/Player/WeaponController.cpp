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

	GetReference();
	SetupInput();
	ApplyEquipment();
}

void UWeaponController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWeaponController::GetReference()
{
	// CharacterBaseからCustomASCとCharacterを取得
	if (const TObjectPtr<ACharacterBase> OwnerCharacterBase = Cast<ACharacterBase>(GetOwner()))
	{
		if (!((CustomAbilitySystemComponent = OwnerCharacterBase->GetAbilitySystemComponent())))
		{
			UE_LOG(LogTemp, Error, TEXT("null CustomAbilitySystemComponent"));
		}

		OwnerCharacter = OwnerCharacterBase;
	}
	else UE_LOG(LogTemp, Error, TEXT("OwnerがCharacterBaseではない"));
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
			EnhancedInputComponent->BindAction(SheathingOfSwordInput, ETriggerEvent::Started, this, &UWeaponController::SheathingOfSword);
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

void UWeaponController::SheathingOfSword()
{
	// todo:状態をTagで持ってAbilityのほうで発動判定してもいい
	if (bIsDrawing) CustomAbilitySystemComponent->SaveTagTryActivateAbilities(SheathingOfSwordInputTag);
}

void UWeaponController::DrawingWeapon()
{
	// 抜刀ソケットの切替
	AttachWeaponToMesh(OwnerCharacter->GetMesh(), WeaponActor->DrawingAttachSocketName, WeaponActor->DrawingAttachPivot);
	// 状態の切替
	bIsDrawing = true;
	// 納刀状態のTagを削除
	CustomAbilitySystemComponent->RemoveTagsWithParent(WeaponSheathedStateTag);
	// 抜刀状態のTagを付与
	CustomAbilitySystemComponent->AddLooseGameplayTag(WeaponDrawnStateTag);
}

void UWeaponController::SheathingWeapon()
{
	// 納刀ソケットの切替
	AttachWeaponToMesh(OwnerCharacter->GetMesh(), WeaponActor->SheathingAttachSocketName, WeaponActor->SheathingAttachPivot);
	// 状態の切替
	bIsDrawing = false;
	// 抜刀状態のTagを削除
	CustomAbilitySystemComponent->RemoveTagsWithParent(WeaponDrawnStateTag);
	// 納刀状態のTagを付与
	CustomAbilitySystemComponent->AddLooseGameplayTag(WeaponSheathedStateTag);
}

void UWeaponController::AttachWeaponToMesh(USkeletalMeshComponent* SkeletalMeshComponent, const FName AttachSocketName,
	const USceneComponent* PivotComponent) const
{
	if (!SkeletalMeshComponent || !PivotComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("null component"));
		return;
	}
	
	// ソケットの切替
	if (SkeletalMeshComponent->DoesSocketExist(AttachSocketName))
	{
		WeaponActor->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, AttachSocketName);
		// 位置、回転の補正
		const FRotator RelativeRotate = PivotComponent->GetRelativeRotation().GetInverse();
		WeaponActor->SetActorRelativeRotation(RelativeRotate);
		WeaponActor->SetActorRelativeLocation(RelativeRotate.RotateVector(-PivotComponent->GetRelativeLocation()));
	}
	else UE_LOG(LogTemp, Error, TEXT("not find socket"));
}

void UWeaponController::DealDamage(FHitResult HitResult)
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

void UWeaponController::AnimHitStop(FHitResult HitResult)
{
	TObjectPtr<UAnimInstance> AnimInstance;
	TObjectPtr<UAnimMontage> CurrentMontage;

	// nullチェックと代入
	if (!OwnerCharacter->GetMesh() || !((AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())) || !((CurrentMontage = AnimInstance->GetCurrentActiveMontage()))) return;

	// 再生速度を変更
	AnimInstance->Montage_SetPlayRate(CurrentMontage, StopSpeed);

	// タイマーセット ヒットストップの時間はワールド時間
	FTimerHandle TimerHandle;
	// 明示的にFTimerDelegateをバインドすることでエラー起きないみたい
	FTimerDelegate TimerDelegate;

	// 一定時間後に再生速度を元に戻す
	TimerDelegate.BindLambda([this, AnimInstance, CurrentMontage]()
	{
		if (AnimInstance)
		{
			// 元の再生速度に戻す
			AnimInstance->Montage_SetPlayRate(CurrentMontage, 1.f);
		}
	});

	// デリゲートの実行
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, HitStopDuration, false); // todo ストップ時間の参照
}

void UWeaponController::OnDealtDamage(float Damage, FVector HitPoint)
{
	// 与えたダメージ量を表示するWidgetを作成する
	if (DamageUIClass)
	{
		if (const TObjectPtr<UDamageDisplayWidget> DamageUIInstance = CreateWidget<UDamageDisplayWidget>(GetWorld(), DamageUIClass))
		{
			DamageUIInstance->AddToViewport();

			if (const TObjectPtr<APlayerController> PlayerController = Cast<APlayerController>(OwnerCharacter->Controller))
			{
				DamageUIInstance->InitDamageDisplay(Damage, HitPoint, PlayerController);
			}
		}
	}
}

void UWeaponController::ApplyEquipment()
{
	if (!Equipment.Weapon)
	{
		UE_LOG(LogTemp, Error, TEXT("not setting weapon"))
		return;
	}
	
	// 武器をOwnerPlayerに持たせる
	// スポーンのパラメーター
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();  // スポーンしたアクターのオーナーを設定
	SpawnParams.Instigator = OwnerCharacter;  // スポーンしたアクターのインスティゲーターを設定

	// アクターを生成
	WeaponActor = GetWorld()->SpawnActor<AWeaponBase>(Equipment.Weapon, SpawnParams);

	// 初期は納刀
	SheathingWeapon();

	if (CustomAbilitySystemComponent)
	{
		// 武器のAbilityをPlayerに持たせる
		for (auto Ability : WeaponActor->AttackAbilities)
		{
			CustomAbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability.GetDefaultObject(), 0, -1));
		}
	}
	else UE_LOG(LogTemp, Warning, TEXT("UWeaponController::ApplyEquipment : CustomAbilitySystemComponent is nullptr"));

	WeaponActor->OnHitAttack.AddDynamic(this, &UWeaponController::DealDamage);
	WeaponActor->OnHitAttack.AddDynamic(this, &UWeaponController::AnimHitStop);
}

