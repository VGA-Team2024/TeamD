#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/ScriptMacros.h"

#include "AtomAisacPatch.generated.h"

// Forward Definitions
class UAtomConfig;
class UAtomModulatorBase;
struct FAtomAisacControlSettings;

/** Struct that define an AISAC control. */
USTRUCT(BlueprintType)
struct CRIWARECORE_API FAtomAisacControl
{
	GENERATED_BODY()

public:

	FAtomAisacControl()
		: ID(INDEX_NONE)
	{}

	FAtomAisacControl(FName InName, int32 InID)
		: Name(InName)
		, ID(InID)
	{}

	bool operator==(const FAtomAisacControl& Other) const
	{
		return ID == Other.ID;
	}

	bool operator!=(const FAtomAisacControl& Other) const
	{
		return !(*this == Other);
	}

	/** Name of the AISAC control. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AISAC")
	FName Name;

	/** Unique Atom Craft ID of the AISAC control. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AISAC")
	int32 ID;
};

inline uint32 GetTypeHash(FAtomAisacControl A) { return static_cast<uint32>(A.ID); }

/** Class to fully define an AISAC patch. */
UCLASS(HideCategories = Object, BlueprintType)
class CRIWARECORE_API UAtomAisacPatch
	: public UObject
{
	GENERATED_BODY()

public:

	void Init(FName InName, bool InDefaultControlFlag, float InDefaultControlValue, const FAtomAisacControl& InControl)
	{
		Name = InName;
		DefaultControlFlag = InDefaultControlFlag;
		DefaultControlValue = InDefaultControlValue;
		Control = InControl;
	}

	/** Name of AISAC patch. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AISAC")
	FName Name;

	/** Flag wether a control value is specified. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AISAC")
	bool DefaultControlFlag;

	/** Default control value when specified. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AISAC")
	float DefaultControlValue;

	/** The control used */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AISAC")
	FAtomAisacControl Control;

	/** Gets the Atom configuaration this AISAC patch belong to. */
	UFUNCTION(BlueprintCallable, Category = "Config")
	UAtomConfig* GetConfig() const;
};

/** class for internal parameters */
struct FAtomAisacControlParameterInfo
{
public:

	FAtomAisacControl Control;

	float Value;
};
