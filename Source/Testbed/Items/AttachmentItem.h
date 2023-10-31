#pragma once

#include "CoreMinimal.h"
#include "ItemActor.h"
#include "AttachmentItem.generated.h"

UCLASS()
class TESTBED_API AAttachmentItem : public AItemActor {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void ToggleAttachmentState(bool State);

};
