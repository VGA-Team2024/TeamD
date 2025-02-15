#include "Character/Player/PlayerItemManager.h"

#include <rapidjson/document.h>

#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/Player/PlayerCharacter.h"
#include "Framework/CustomFramework.h"

UPlayerItemManager::UPlayerItemManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerItemManager::BeginPlay()
{
	Super::BeginPlay();

	SetupInput();

	// アイテムに参照を渡すため
	CustomAbilitySystemComponent = Cast<APlayerCharacter>(GetOwner())->GetAbilitySystemComponent();
	if (!CustomAbilitySystemComponent) UE_LOG(LogTemp, Error, TEXT("customASC is not player"));
}

void UPlayerItemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPlayerItemManager::SetupInput()
{
	// PlayerControllerの取得
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (GetOwner()->InputComponent)
	{
		// アクションをバインドする
		if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(GetOwner()->InputComponent))
		{
			EnhancedInputComponent->BindAction(UseItemInput, ETriggerEvent::Started, this, &UPlayerItemManager::UseItem);
		}

		// Input Mapping Contextを登録する
		if (PlayerController)
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(ItemMappingContext, 0);
			}
		}
	}
}

void UPlayerItemManager::UseItem()
{
	// todo:アイテムの使用　納刀状態の判定はここかAbilityかどっちでするか
	UE_LOG(LogTemp, Log, TEXT("call use item"));

	// index判定
	if (ItemsInPossession.IsValidIndex(SelectedItemIndex))
	{
		if (!IsValid(ItemsInPossession[SelectedItemIndex]))
		{
			LOG_INFO(Error, "not valid");
			return;
		}
	
		// アイテムの使用を呼び出す
		ItemsInPossession[SelectedItemIndex]->UseItem();
	}
}

void UPlayerItemManager::ItemInfoChangedBroadcast() const
{
	OnItemInfoChanged.Broadcast(ItemsInPossession, SelectedItemIndex);
}

void UPlayerItemManager::AddItem(TSubclassOf<UItemBase> ItemToAdd, int32 Num)
{
	// すでに持っているアイテムかをループしてみる
	for (auto ItemInPossession : ItemsInPossession)
	{
		if (ItemInPossession && ItemInPossession.GetClass() == ItemToAdd)
		{
			ItemInPossession->AddStack(Num);
			return;
		}
	}

	// 持ってないアイテムなので新しく追加する
    TObjectPtr<UItemBase> NewItem = NewObject<UItemBase>(this, ItemToAdd);
	NewItem->Init(CustomAbilitySystemComponent);
	NewItem->OnItemStackChanged.AddLambda([this](int32 Value)
	{
		ItemInfoChangedBroadcast();
	});
	ItemsInPossession.Add(NewItem);
	NewItem->AddStack(Num);
}
