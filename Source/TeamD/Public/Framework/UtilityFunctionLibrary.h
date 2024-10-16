// UtilityFunctionLibrary.h

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UtilityFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class TEAMD_API UUtilityFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable,
		meta=(WorldContext="WorldContextObject", AdvancedDisplay = "2", DisplayName =
			"Open Level (by Object Reference) and Set GameMode"), Category="Game")
	static void OpenLevelBySoftObjectPtrAndSetGameMode(
		const UObject* WorldContextObject,
		const TSoftObjectPtr<UWorld> Level,
		const TSubclassOf<AGameModeBase> GameModeClass, // 修正
		bool bAbsolute = true);
};
