#include "Character/Player/ItemBase.h"

void UItemBase::UseItem_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("item base : use item"));
}

void UItemBase::ApplyItemEffect_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("item base : apply item effect"));
}

void UItemBase::AddStack(int32 Value)
{
	CurrentStackNum = FMath::Min(CurrentStackNum + Value, MaxStackNum);
}

int32 UItemBase::GetMaxStack() const { return MaxStackNum; }

int32 UItemBase::GetCurrentStack() const { return CurrentStackNum; }
