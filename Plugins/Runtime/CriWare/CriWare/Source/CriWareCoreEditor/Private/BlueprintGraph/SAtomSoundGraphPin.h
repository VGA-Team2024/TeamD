
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphNodeUtils.h"
#include "EdGraphUtilities.h"
#include "K2Node.h"
#include "SGraphPin.h"

// Forward Definitions
class UAtomSoundBase;
class UAtomSoundGraphPinObject;

class SAtomSoundGraphPin : public SGraphPin, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SAtomSoundGraphPin) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj);

	//~ Begin SGraphPin Interface
	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override;
	//~ End SGraphPin Interface

	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override
	{
		return TEXT("SAtomSoundGraphPin");
	}
	//~ End FGCObject Interface

private:

	bool OnSelectionChanged(UObject* SoundObject);
	bool SetValue(UAtomSoundBase* Sound);
	UAtomSoundBase* GetValue() const;

	TObjectPtr<UAtomSoundGraphPinObject> SoundPropertyHolderObject;
};
