// UtilityFunctionLibrary.cpp

#include "Framework/UtilityFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h" // AGameModeBase のインクルード


void UUtilityFunctionLibrary::OpenLevelBySoftObjectPtrAndSetGameMode(const UObject* WorldContextObject,
																	 const TSoftObjectPtr<UWorld> Level,
																	 TSubclassOf<AGameModeBase> GameModeClass, // 修正
																	 bool bAbsolute)
{
	if (WorldContextObject == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("WorldContextObject is null"));
		return;
	}

	if (!Level.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Level is invalid"));
		return;
	}

	FString Options;

	// ゲームモードが有効であれば、オプションに追加
	if (GameModeClass != nullptr)
	{
		FString GameModePath = GameModeClass->GetPathName();
		Options += FString::Printf(TEXT("?game=%s"), *GameModePath);
	}

	// デバッグ用のログ出力
	UE_LOG(LogTemp, Log, TEXT("Opening Level: %s with GameMode: %s"), *Level.GetAssetName(),
		   *GameModeClass->GetPathName());

	// レベルを開く
	UGameplayStatics::OpenLevel(WorldContextObject, FName(*FPackageName::ObjectPathToPackageName(Level.ToString())),
								bAbsolute, Options);
}
