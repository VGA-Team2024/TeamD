#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "GAS/PlayerAttributeSet.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class TEAMD_API APlayerCharacter : public ACharacterBase
{
	GENERATED_BODY()

protected:
	APlayerCharacter();
	
	virtual void BeginPlay() override;
	
//------------------------input------------------------
	
	// MappingContext
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputAction> MoveInput;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputAction> LookInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputAction> DodgeInput;

private:
	// input設定
	void SetupInput();

	// イベントハンドラー関数
	// Move
	void MovePlayer(const FInputActionValue& Value);
	// Look
	void RotateControllerInput(const FInputActionValue& Value);
	// Dodge
	void PressedDodge();
	void ReleasedDodge();
};
