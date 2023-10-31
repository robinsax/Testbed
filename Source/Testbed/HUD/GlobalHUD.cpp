#include "GlobalHUD.h"

#include "../Util.h"

AGlobalHUD::AGlobalHUD() {
    PrimaryActorTick.bCanEverTick = true;
}

void AGlobalHUD::BeginPlay() {
    Super::BeginPlay();

    this->HUDWidget = CreateWidget<UGlobalHUDWidget>(this->GetWorld(), this->HUDWidgetType);
    this->InventoryWidget = CreateWidget<UGlobalHUDWidget>(this->GetWorld(), this->InventoryWidgetType);

    this->HUDWidget->BindTo(this);
    this->HUDWidget->AddToViewport();
    this->InventoryWidget->BindTo(this);
    this->InventoryWidget->AddToViewport();

    this->HUDWidget->SetVisibility(ESlateVisibility::Visible);
}

void AGlobalHUD::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    EPlayerControlMode Mode = AGlobalPlayerController::GetMode(this->GetWorld());

    this->HUDWidget->SetVisibility(Mode == EPlayerControlMode::Normal ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    this->InventoryWidget->SetVisibility(Mode == EPlayerControlMode::Inventory ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

ABaseRobot* AGlobalHUD::GetRobot() {
    return AGlobalPlayerController::Get(this->GetWorld())->GetRobot();
}
