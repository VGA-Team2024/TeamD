#pragma once

#include "Containers/Array.h"
#include "UObject/Interface.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"

#include "AtomParameter.generated.h"

UENUM()
enum class EAtomParameterType : uint8
{
	// 'Default' results in behavior that is resolved
	// based on the system interpreting it. 
	// AtomSoundCues use type associated with a given 
	// parameter name, whereas MetaSounds
	// only use the last typed value set.
	None UMETA(DisplayName = "Default"),

	// Boolean value
	Boolean,

	// Integer value
	Integer,

	// Float value
	Float,

	// String value
	String,

	// Object value (for MetaSound system)
	//Object,

	// Aisac Value (for AtomSoundCue system)
	Aisac = Float UMETA(DisplayName = "AISAC"),

	// Array of default initialized values (for MetaSound system)
	// Hidden for now as no parameter types exist that support default construction
	NoneArray UMETA(Hidden, DisplayName = "Default (Array)"),

	// Array of boolean values (for MetaSound system)
	BooleanArray UMETA(DisplayName = "Boolean (Array)"),

	// Array of integer values (for MetaSound system)
	IntegerArray UMETA(DisplayName = "Integer (Array)"),

	// Array of float values (for MetaSound system)
	FloatArray UMETA(DisplayName = "Float (Array)"),

	// Array of string values (for MetaSound system)
	StringArray UMETA(DisplayName = "String (Array)"),

	// Array of object values (for MetaSound system)
	//ObjectArray UMETA(DisplayName = "Object (Array)"),

	COUNT UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct CRIWARECORE_API FAtomParameter
{
	GENERATED_BODY()

public:

	FAtomParameter() = default;

	FAtomParameter(FName InName)
		: ParamName(InName)
	{
	}

	FAtomParameter(FName InName, float InValue)
		: ParamName(InName)
		, FloatParam(InValue)
		, ParamType(EAtomParameterType::Float)
	{
	}

	FAtomParameter(FName InName, bool InValue)
		: ParamName(InName)
		, BoolParam(InValue)
		, ParamType(EAtomParameterType::Boolean)
	{
	}

	FAtomParameter(FName InName, int32 InValue)
		: ParamName(InName)
		, IntParam(InValue)
		, ParamType(EAtomParameterType::Integer)
	{
	}

	/*FAtomParameter(FName InName, UObject* InValue)
		: ParamName(InName)
		, ObjectParam(InValue)
		, ParamType(EAtomParameterType::Object)
	{
	}*/

	FAtomParameter(FName InName, const FString& InValue)
		: ParamName(InName)
		, StringParam(InValue)
		, ParamType(EAtomParameterType::String)
	{
	}

	FAtomParameter(FName InName, const TArray<float>& InValue)
		: ParamName(InName)
		, ArrayFloatParam(InValue)
		, ParamType(EAtomParameterType::FloatArray)
	{
	}

	FAtomParameter(FName InName, TArray<float>&& InValue)
		: ParamName(InName)
		, ArrayFloatParam(MoveTemp(InValue))
		, ParamType(EAtomParameterType::FloatArray)
	{
	}

	FAtomParameter(FName InName, const TArray<bool>& InValue)
		: ParamName(InName)
		, ArrayBoolParam(InValue)
		, ParamType(EAtomParameterType::BooleanArray)
	{
	}

	FAtomParameter(FName InName, TArray<bool>&& InValue)
		: ParamName(InName)
		, ArrayBoolParam(MoveTemp(InValue))
		, ParamType(EAtomParameterType::BooleanArray)
	{
	}

	FAtomParameter(FName InName, const TArray<int32>& InValue)
		: ParamName(InName)
		, ArrayIntParam(InValue)
		, ParamType(EAtomParameterType::IntegerArray)
	{
	}

	FAtomParameter(FName InName, TArray<int32>&& InValue)
		: ParamName(InName)
		, ArrayIntParam(MoveTemp(InValue))
		, ParamType(EAtomParameterType::IntegerArray)
	{
	}

	/*FAtomParameter(FName InName, const TArray<UObject*>& InValue)
		: ParamName(InName)
		, ArrayObjectParam(InValue)
		, ParamType(EAtomParameterType::ObjectArray)
	{
	}

	FAtomParameter(FName InName, TArray<UObject*>&& InValue)
		: ParamName(InName)
		, ArrayObjectParam(MoveTemp(InValue))
		, ParamType(EAtomParameterType::ObjectArray)
	{
	}*/

	FAtomParameter(FName InName, const TArray<FString>& InValue)
		: ParamName(InName)
		, ArrayStringParam(InValue)
		, ParamType(EAtomParameterType::StringArray)
	{
	}

	FAtomParameter(FName InName, TArray<FString>&& InValue)
		: ParamName(InName)
		, ArrayStringParam(MoveTemp(InValue))
		, ParamType(EAtomParameterType::StringArray)
	{
	}

	// Static function to avoid int32 constructor collision
	static FAtomParameter CreateDefaultArray(FName InName, int32 InNum)
	{
		FAtomParameter NewParam(InName, InNum);
		NewParam.ParamType = EAtomParameterType::NoneArray;
		return NewParam;
	}

	// Sets values specified by type field of the given parameter on this parameter. If the provided parameter is set to type 'None', takes all values of the given parameter.
	// bInTakeName = Take the name of the provided parameter.
	// bInTakeType = Take the type of the provided parameter.
	// bInMergeArrayTypes - Appends array(s) for specified type if true, else swaps the local value with that of the provided parameter if false.
	void Merge(const FAtomParameter& InParameter, bool bInTakeName = true, bool bInTakeType = true, bool bInMergeArrayTypes = false);

	// Moves InParams to OutParams that are not already included. For backward compatibility (i.e. SoundCues),
	// if a param is already in OutParams, attempts to merge param values together, but assigns the param the
	// incoming param's type. Currently existing OutParam values left if new value of the same type is provided
	// by InParam.
	static void Merge(TArray<FAtomParameter>&& InParams, TArray<FAtomParameter>& OutParams);

	FAtomParameter(FAtomParameter&& InParameter)
		: ParamName(MoveTemp(InParameter.ParamName))
		, FloatParam(MoveTemp(InParameter.FloatParam))
		, BoolParam(MoveTemp(InParameter.BoolParam))
		, IntParam(MoveTemp(InParameter.IntParam))
		//, ObjectParam(MoveTemp(InParameter.ObjectParam))
		, StringParam(MoveTemp(InParameter.StringParam))
		, ArrayFloatParam(MoveTemp(InParameter.ArrayFloatParam))
		, ArrayBoolParam(MoveTemp(InParameter.ArrayBoolParam))
		, ArrayIntParam(MoveTemp(InParameter.ArrayIntParam))
		//, ArrayObjectParam(MoveTemp(InParameter.ArrayObjectParam))
		, ArrayStringParam(MoveTemp(InParameter.ArrayStringParam))
		, ParamType(MoveTemp(InParameter.ParamType))
		, TypeName(MoveTemp(InParameter.TypeName))
		//, ObjectProxies(MoveTemp(InParameter.ObjectProxies))		
	{
	}

	FAtomParameter(const FAtomParameter& InParameter)
		: ParamName(InParameter.ParamName)
		, FloatParam(InParameter.FloatParam)
		, BoolParam(InParameter.BoolParam)
		, IntParam(InParameter.IntParam)
		//, ObjectParam(InParameter.ObjectParam)
		, StringParam(InParameter.StringParam)
		, ArrayFloatParam(InParameter.ArrayFloatParam)
		, ArrayBoolParam(InParameter.ArrayBoolParam)
		, ArrayIntParam(InParameter.ArrayIntParam)
		//, ArrayObjectParam(InParameter.ArrayObjectParam)
		, ArrayStringParam(InParameter.ArrayStringParam)
		, ParamType(InParameter.ParamType)
		, TypeName(InParameter.TypeName)
	{
		/*ObjectProxies.Reset();
		for (const Audio::IProxyDataPtr& DataPtr : InParameter.ObjectProxies)
		{
			ObjectProxies.Emplace(DataPtr->Clone());
		}*/
	}

	FAtomParameter& operator=(const FAtomParameter& InParameter)
	{
		ParamName = InParameter.ParamName;
		FloatParam = InParameter.FloatParam;
		BoolParam = InParameter.BoolParam;
		IntParam = InParameter.IntParam;
		//ObjectParam = InParameter.ObjectParam;
		StringParam = InParameter.StringParam;
		ArrayFloatParam = InParameter.ArrayFloatParam;
		ArrayBoolParam = InParameter.ArrayBoolParam;
		ArrayIntParam = InParameter.ArrayIntParam;
		//ArrayObjectParam = InParameter.ArrayObjectParam;
		ArrayStringParam = InParameter.ArrayStringParam;
		ParamType = InParameter.ParamType;
		TypeName = InParameter.TypeName;

		/*ObjectProxies.Reset();
		for (const Audio::IProxyDataPtr& DataPtr : InParameter.ObjectProxies)
		{
			ObjectProxies.Emplace(DataPtr->Clone());
		}*/

		return *this;
	}

	FAtomParameter& operator=(FAtomParameter&& InParameter)
	{
		ParamName = MoveTemp(InParameter.ParamName);
		FloatParam = MoveTemp(InParameter.FloatParam);
		BoolParam = MoveTemp(InParameter.BoolParam);
		IntParam = MoveTemp(InParameter.IntParam);
		//ObjectParam = MoveTemp(InParameter.ObjectParam);
		StringParam = MoveTemp(InParameter.StringParam);
		ArrayFloatParam = MoveTemp(InParameter.ArrayFloatParam);
		ArrayBoolParam = MoveTemp(InParameter.ArrayBoolParam);
		ArrayIntParam = MoveTemp(InParameter.ArrayIntParam);
		//ArrayObjectParam = MoveTemp(InParameter.ArrayObjectParam);
		ArrayStringParam = MoveTemp(InParameter.ArrayStringParam);
		ParamType = MoveTemp(InParameter.ParamType);
		TypeName = MoveTemp(InParameter.TypeName);

		//ObjectProxies = MoveTemp(InParameter.ObjectProxies);

		return *this;
	}

	// Name of the parameter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName="Name"), Category = AtomParameter)
	FName ParamName;

	// Float value of parameter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Value (Float)", DisplayAfter = "ParamType", EditConditionHides, EditCondition = "ParamType == EAtomParameterType::None || ParamType == EAtomParameterType::Float"), Category = AtomParameter)
	float FloatParam = 0.f;

	// Boolean value of parameter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Value (Bool)", DisplayAfter = "ParamType", EditConditionHides, EditCondition = "ParamType == EAtomParameterType::None || ParamType == EAtomParameterType::Boolean"), Category = AtomParameter)
	bool BoolParam = false;

	// Integer value of parameter. If set to 'Default Construct', value is number of array items to construct.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Value (Int)", DisplayAfter = "ParamType", EditConditionHides, EditCondition = "ParamType == EAtomParameterType::None || ParamType == EAtomParameterType::Integer || ParamType == EAtomParameterType::NoneArray"), Category = AtomParameter)
	int32 IntParam = 0;

	// Object value of parameter
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Value (Object)", DisplayAfter = "ParamType", EditConditionHides, EditCondition = "ParamType == EAtomParameterType::None || ParamType == EAtomParameterType::Object"), Category = AtomParameter)
	//TObjectPtr<UObject> ObjectParam = nullptr;

	// String value of parameter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Value (String)", DisplayAfter = "ParamType", EditConditionHides, EditCondition = "ParamType == EAtomParameterType::String"), Category = AtomParameter)
	FString StringParam;

	// Array Float value of parameter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Value (Float Array)", DisplayAfter = "ParamType", EditConditionHides, EditCondition = "ParamType == EAtomParameterType::FloatArray"), Category = AtomParameter)
	TArray<float> ArrayFloatParam;

	// Boolean value of parameter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Value (Bool Array)", DisplayAfter = "ParamType", EditConditionHides, EditCondition = "ParamType == EAtomParameterType::BooleanArray"), Category = AtomParameter)
	TArray<bool> ArrayBoolParam;

	// Integer value of parameter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Value (Int Array)", DisplayAfter = "ParamType", EditConditionHides, EditCondition = "ParamType == EAtomParameterType::IntegerArray"), Category = AtomParameter)
	TArray<int32> ArrayIntParam;

	// Object value of parameter
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Value (Object Array)", DisplayAfter = "ParamType", EditConditionHides, EditCondition = "ParamType == EAtomParameterType::ObjectArray"), Category = AtomParameter)
	//TArray<TObjectPtr<UObject>> ArrayObjectParam;

	// String value of parameter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Value (String Array)", DisplayAfter = "ParamType", EditConditionHides, EditCondition = "ParamType == EAtomParameterType::StringArray"), Category = AtomParameter)
	TArray<FString> ArrayStringParam;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Type"), Category = AtomParameter)
	EAtomParameterType ParamType = EAtomParameterType::None;

	// Optional TypeName used to describe what constructed type this parameter should be initializing.
	UPROPERTY()
	FName TypeName;

	// Object proxies to be generated when parameter is passed to the AtomThread to represent ObjectParam/ArrayObjectParam safely
	//TArray<Audio::IProxyDataPtr> ObjectProxies;

	// Common find algorithm for default/legacy parameter system
	static const FAtomParameter* FindParam(const TArray<FAtomParameter>& InParams, FName InParamName)
	{
		if (!InParamName.IsNone())
		{
			for (const FAtomParameter& ExistingParam : InParams)
			{
				if (ExistingParam.ParamName == InParamName)
				{
					return &ExistingParam;
				}
			}
		}

		return nullptr;
	}

	// Common find & add algorithm for default/legacy parameter system.
	static FAtomParameter* FindOrAddParam(TArray<FAtomParameter>& OutParams, FName InParamName)
	{
		FAtomParameter* Param = nullptr;
		if (InParamName.IsNone())
		{
			return Param;
		}

		for (FAtomParameter& ExistingParam : OutParams)
		{
			if (ExistingParam.ParamName == InParamName)
			{
				Param = &ExistingParam;
				break;
			}
		}

		if (!Param)
		{
			Param = &OutParams.AddDefaulted_GetRef();
			Param->ParamName = InParamName;
		}

		return Param;
	}
};

/*
 * Cue parameters (Selector ...)
 *****************************************************************************/

 /** Struct that define an Atom selector. */
USTRUCT(BlueprintType, meta = (ToolTip = "Describe an Atom selector with its labels."))
struct CRIWARECORE_API FAtomSelector
{
	GENERATED_BODY()

public:

	FAtomSelector() {}
	FAtomSelector(FName InName, const TArray<FName>& InLabels) : Name(InName), Labels(InLabels) {}

	FORCEINLINE FName GetName() const { return Name; }
	FORCEINLINE int GetNumLabels() const { return Labels.Num(); }
	FORCEINLINE FName GetLabel(int Index) const { return Labels.IsValidIndex(Index) ? Labels[Index] : NAME_None; }
	FORCEINLINE int FindLabel(FName Label) const { return Labels.Find(Label); }
	FORCEINLINE bool ContainLabel(FName Label) const { return Labels.Contains(Label); }

protected:

	/** Selector Name. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atom", meta = (ToolTip = "The selector name."))
	FName Name;

	/** Selector Labels */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atom", meta = (ToolTip = "The list of labels contained in this selector."))
	TArray<FName> Labels;
};

/** Struct that define an Atom selector label parameter. */
USTRUCT(BlueprintType, meta = (ToolTip = "Structure to setup a selector label as parameter."))
struct CRIWARECORE_API FAtomSelectorParam
{
	GENERATED_BODY()

public:

	FAtomSelectorParam();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Selector)
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Selector)
	FName Label;
};

