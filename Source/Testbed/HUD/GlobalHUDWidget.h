#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "GlobalHUDWidget.generated.h"

class AGlobalHUD;

UCLASS()
class UGlobalHUDWidget : public UUserWidget {
	GENERATED_BODY()

private:
	AGlobalHUD* Parent;

public:
	void BindTo(AGlobalHUD* NewParent);

	UFUNCTION(BlueprintCallable)
	AGlobalHUD* GetParentHUD();
};
