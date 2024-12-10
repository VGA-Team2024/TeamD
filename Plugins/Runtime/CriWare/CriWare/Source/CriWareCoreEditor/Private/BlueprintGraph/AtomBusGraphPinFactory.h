
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphNodeUtils.h"
#include "EdGraphUtilities.h"
#include "K2Node.h"

#include "Atom/AtomBus.h" 
#include "SAtomBusGraphPin.h"

class FAtomBusGraphPinFactory : public FGraphPanelPinFactory
{
	virtual TSharedPtr<class SGraphPin> CreatePin(class UEdGraphPin* InPin) const override
	{
		const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
		/* Check if pin is struct, and then check if that pin is of struct type we want customize. */
		if (InPin->PinType.PinCategory == K2Schema->PC_Object
			&& InPin->PinType.PinSubCategoryObject == UAtomBus::StaticClass())
		{
			return SNew(SAtomBusGraphPin, InPin); //and return our customized pin widget ;).
		}
		return nullptr;
	}
};
