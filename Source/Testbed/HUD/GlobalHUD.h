#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "../Meta/GlobalPlayerController.h"
#include "GlobalHUDWidget.h"

#include "GlobalHUD.generated.h"

UCLASS()
class AGlobalHUD : public AHUD {
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category="Widgets")
	TSubclassOf<UGlobalHUDWidget> HUDWidgetType;
	UPROPERTY(EditDefaultsOnly, Category="Widgets")
	TSubclassOf<UGlobalHUDWidget> InventoryWidgetType;

	UGlobalHUDWidget* HUDWidget;
	UGlobalHUDWidget* InventoryWidget;

public:
	AGlobalHUD();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	ABaseRobot* GetRobot();
	
};
