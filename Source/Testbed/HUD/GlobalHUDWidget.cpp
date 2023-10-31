#include "GlobalHUDWidget.h"

void UGlobalHUDWidget::BindTo(AGlobalHUD* NewParent) {
    this->Parent = NewParent;
}

AGlobalHUD* UGlobalHUDWidget::GetParentHUD() {
    return this->Parent;
}
