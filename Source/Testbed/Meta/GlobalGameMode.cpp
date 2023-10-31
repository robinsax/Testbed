#include "GlobalGameMode.h"

#include "GlobalPlayerController.h"
#include "../HUD/GlobalHUD.h"

AGlobalGameMode::AGlobalGameMode() {
    this->PlayerControllerClass = AGlobalPlayerController::StaticClass();
}
