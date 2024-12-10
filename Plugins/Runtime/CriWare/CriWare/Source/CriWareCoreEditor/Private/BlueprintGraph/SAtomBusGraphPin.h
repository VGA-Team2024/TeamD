
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphNodeUtils.h"
#include "EdGraphUtilities.h"
#include "K2Node.h"
#include "SGraphPin.h"

// Forward Definitions
class UAtomBus;
class UAtomBusGraphPinObject;

class SAtomBusGraphPin : public SGraphPin, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SAtomBusGraphPin) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj);

	//~ Begin SGraphPin Interface
	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override;
	//~ End SGraphPin Interface

	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override
	{
		return TEXT("SAtomBusGraphPin");
	}
	//~ End FGCObject Interface

private:

	bool OnSelectionChanged(UObject* BusObject);
	bool SetValue(UAtomBus* Bus);
	UAtomBus* GetValue() const;

	TObjectPtr<UAtomBusGraphPinObject> BusPropertyHolderObject;
};
