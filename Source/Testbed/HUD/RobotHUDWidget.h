#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "RobotHUDWidget.generated.h"

class ABaseRobot;

UCLASS()
class TESTBED_API URobotHUDWidget : public UUserWidget {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	ABaseRobot* Robot;

};
