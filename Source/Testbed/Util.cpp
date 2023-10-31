#include "Util.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

DEFINE_LOG_CATEGORY(AA_Sanity);

FString _UtilGetNetModeName() {
    AGameModeBase* GameMode = GWorld->GetAuthGameMode();
    if (IsValid(GameMode)) {
        if (GameMode->GetNetMode() == NM_DedicatedServer) {
            return TEXT("Server");
        }
    }

    return TEXT("Client");
}

int _UtilGetNetId() {
    APlayerController* Controller = GWorld->GetFirstPlayerController();
    if (IsValid(Controller)) {
        APlayerState* State = Controller->PlayerState;
        if (IsValid(State)) return State->GetPlayerId();
    }

    return -1;
}
