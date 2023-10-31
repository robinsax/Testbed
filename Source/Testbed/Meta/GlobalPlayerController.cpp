#include "GlobalPlayerController.h"

#include "Kismet/GameplayStatics.h"

#include "../Util.h"
#include "../HUD/GlobalHUD.h"
#include "../Items/ItemActor.h"
#include "../Robots/HumanoidRobot.h"

AGlobalPlayerController* AGlobalPlayerController::Get(UWorld* World) {
    return Cast<AGlobalPlayerController>(GEngine->GetFirstLocalPlayerController(World));
}

EPlayerControlMode AGlobalPlayerController::GetMode(UWorld* World) {
    AGlobalPlayerController* Controller = AGlobalPlayerController::Get(World);
    if (!IsValid(Controller)) return EPlayerControlMode::None;

    return Controller->GetMode();
}

void AGlobalPlayerController::BeginPlay() {
    Super::BeginPlay();

    this->Mode = EPlayerControlMode::Normal;

    this->InventoryWorldCursor = this->GetWorld()->SpawnActor<AActor>(this->InventoryWorldCursorType, FVector::ZeroVector, FRotator(0.0f, 0.0f, 0.0f));
    this->InventoryWorldCursor->SetActorHiddenInGame(true);

    // TODO: no lol.    
    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsWithTag(this->GetWorld(), FName(TEXT("OutlineVolumeNormal")), Actors);
    this->NormalOutlineVolume = Cast<APostProcessVolume>(Actors[0]);
    Actors.Reset();
    UGameplayStatics::GetAllActorsWithTag(this->GetWorld(), FName(TEXT("OutlineVolumeDamage")), Actors);
    this->DamageOutlineVolume = Cast<APostProcessVolume>(Actors[0]);
    
    if (!IsValid(this->NormalOutlineVolume) || !IsValid(this->DamageOutlineVolume)) {
        SANITY_WARN("PlayerController couldn't bind PPVs");
    }
}

void AGlobalPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();

	this->InputComponent->BindAction("Inventory", IE_Pressed, this, &AGlobalPlayerController::InputStartViewInventory);
	this->InputComponent->BindAction("Inventory", IE_Released, this, &AGlobalPlayerController::InputEndViewInventory);
    
	this->InputComponent->BindAction("Mouse1", IE_Pressed, this, &AGlobalPlayerController::InputStartMouse1);
	this->InputComponent->BindAction("Mouse1", IE_Released, this, &AGlobalPlayerController::InputEndMouse1);

	this->InputComponent->BindAction("Mouse2", IE_Pressed, this, &AGlobalPlayerController::InputStartMouse2);
	this->InputComponent->BindAction("Mouse2", IE_Released, this, &AGlobalPlayerController::InputEndMouse2);
}

void AGlobalPlayerController::Tick(float DeltaTime) {
    ABaseRobot* Robot = this->GetRobot();
    if (!IsValid(Robot)) return;

    bool WasInInventory = this->InInventory;
    this->InInventory = this->Mode == EPlayerControlMode::Inventory;

    AHumanoidRobot* HumanoidRobot = Cast<AHumanoidRobot>(Robot);
    if (IsValid(HumanoidRobot)) {
        this->ShowPersistentDamage = HumanoidRobot->IsCheckingStatus();
    }
    else {
        this->ShowPersistentDamage = false;
    }
    if (IsValid(this->NormalOutlineVolume)) this->NormalOutlineVolume->bEnabled = !this->ShowPersistentDamage;
    if (IsValid(this->DamageOutlineVolume)) this->DamageOutlineVolume->bEnabled = this->ShowPersistentDamage;

    if (!IsValid(this->InventoryWorldCursor)) return;

    this->CurrentInventorySlot = nullptr;

    if (this->InInventory && !WasInInventory) {
        int ScreenWidth, ScreenHeight;
        this->GetViewportSize(ScreenWidth, ScreenHeight);

        this->SetMouseLocation(ScreenWidth/2, ScreenHeight/2);
    }

    TArray<AActor*> IgnoreActors;

    this->InventoryWorldCursor->SetActorHiddenInGame(!this->InInventory);
    if (!this->InInventory) {
        this->CurrentInventoryItem = nullptr;
        return;
    }

    FScreenRaycastResult MainHit = this->ScreenRaycast(this->NormalizedMousePosition(), 400.0f, IgnoreActors, ECC_GameTraceChannel4);
    this->InventoryWorldCursor->SetActorLocation(MainHit.EndLocation);

    if (!this->MovingItem) {
        this->CurrentInventoryItem = nullptr;

        if (IsValid(MainHit.Actor)) {
            AItemActor* Item = Cast<AItemActor>(MainHit.Actor);

            if (IsValid(Item)) {
                this->CurrentInventoryItem = Item;
            }
        }

        return;
    }

    FScreenRaycastResult SlotHit = this->ScreenRaycast(this->NormalizedMousePosition(), 400.0f, TArray<AActor*>(), ECC_GameTraceChannel3);
    UInventorySlotComponent* Slot = Cast<UInventorySlotComponent>(SlotHit.Component);
    if (IsValid(Slot) && Slot->CanHoldItem(this->CurrentInventoryItem)) {
        this->InventoryWorldCursor->SetActorLocation(SlotHit.EndLocation);

        this->CurrentInventorySlot = Slot;
    }
}

FScreenRaycastResult AGlobalPlayerController::ScreenRaycast(FVector2D NormScreenPosition, float MaxDistance, TArray<AActor*> IgnoreActors, ECollisionChannel Channel) {
    if (MaxDistance < 0) MaxDistance = 10000.0f;
	int ScreenWidth, ScreenHeight;
	this->GetViewportSize(ScreenWidth, ScreenHeight);

    FCollisionQueryParams QueryParams;
    for (int I = 0; I < IgnoreActors.Num(); I++) {
        QueryParams.AddIgnoredActor(IgnoreActors[I]);
    }

	FVector WorldLocation;
	FVector WorldDirection;
	this->DeprojectScreenPositionToWorld(ScreenWidth * NormScreenPosition.X, ScreenHeight * NormScreenPosition.Y, WorldLocation, WorldDirection);
	
	FVector RayEnd = WorldLocation + WorldDirection * MaxDistance;

	FHitResult HitResult;
	this->GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, RayEnd, Channel, QueryParams);

	FScreenRaycastResult Result;
	if (!HitResult.ImpactPoint.IsZero()) Result.EndLocation = HitResult.ImpactPoint;
	else Result.EndLocation = RayEnd;

	Result.Actor = HitResult.GetActor();
    Result.Component = HitResult.GetComponent();

	return Result;
}

FScreenRaycastResult AGlobalPlayerController::ScreenRaycast(FVector2D NormScreenPosition, float MaxDistance) {
    return this->ScreenRaycast(NormScreenPosition, MaxDistance, TArray<AActor*>(), ECC_Visibility);
}

FVector2D AGlobalPlayerController::NormalizedMousePosition() {
    float MouseX;
    float MouseY;
    this->GetMousePosition(MouseX, MouseY);

    int ViewportX;
    int ViewportY;
    this->GetViewportSize(ViewportX, ViewportY);

    return FVector2D(MouseX / ViewportX, MouseY / ViewportY);
}

EPlayerControlMode AGlobalPlayerController::GetMode() {
    return this->Mode;
}

AGlobalHUD* AGlobalPlayerController::GetGlobalHUD() {
    return Cast<AGlobalHUD>(this->GetHUD());
}

ABaseRobot* AGlobalPlayerController::GetRobot() {
    return Cast<ABaseRobot>(this->GetPawn());
}

void AGlobalPlayerController::InputStartViewInventory() {
    ABaseRobot* Robot = this->GetRobot();
    if (!IsValid(Robot)) return;

    this->Mode = EPlayerControlMode::Inventory;
}

void AGlobalPlayerController::InputEndViewInventory() {
    ABaseRobot* Robot = this->GetRobot();
    if (!IsValid(Robot)) return;

    this->Mode = EPlayerControlMode::Normal;
    this->MovingItem = false;
    this->CurrentInventoryItem = nullptr;
    this->CurrentInventorySlot = nullptr;
}

void AGlobalPlayerController::InputStartMouse1() {
    if (this->Mode == EPlayerControlMode::Normal) {
        Cast<AHumanoidRobot>(this->GetRobot())->InputStartFire(); // TODO: Yikes.
        return;
    }

    if (!IsValid(this->CurrentInventoryItem)) return;

    this->MovingItem = true;
}

void AGlobalPlayerController::InputEndMouse1() {
    if (this->Mode == EPlayerControlMode::Normal) {
        Cast<AHumanoidRobot>(this->GetRobot())->InputEndFire(); // TODO: Yikes.
        return;
    }

    ABaseRobot* Robot = this->GetRobot();

    if (!this->MovingItem) return;
    if (IsValid(this->CurrentInventorySlot)) {
        if (Robot->GetInventory()->ContainsItem(this->CurrentInventoryItem)) {
            Robot->MoveItem(this->CurrentInventoryItem, this->CurrentInventorySlot);
        }
        else {
            Robot->TakeItemToSlot(this->CurrentInventoryItem, this->CurrentInventorySlot);
        }
    }
    else if ((this->InventoryWorldCursor->GetActorLocation() - Robot->GetActorLocation()).Size() > 100.0f) {
        Robot->DropItem(this->CurrentInventoryItem);
    }

    this->CurrentInventorySlot = nullptr;
    this->CurrentInventoryItem = nullptr;
    this->MovingItem = false;
}

void AGlobalPlayerController::InputStartMouse2() {
    Cast<AHumanoidRobot>(this->GetRobot())->InputStartAim();
}

void AGlobalPlayerController::InputEndMouse2() {
    if (!IsValid(this->CurrentInventoryItem)) {
        Cast<AHumanoidRobot>(this->GetRobot())->InputEndAim();
        return;
    }

    this->GetRobot()->DropItem(this->CurrentInventoryItem);
    this->CurrentInventorySlot = nullptr;
}

bool AGlobalPlayerController::IsSlotDropTarget(UInventorySlotComponent* Slot) {
    if (this->ShowPersistentDamage) return false;
    if (!this->MovingItem) return false;

    return Slot->GetItem() == nullptr && Slot->CanHoldItem(this->CurrentInventoryItem);
}

bool AGlobalPlayerController::IsItemManageTarget(AItemActor* Item) {
    if (this->ShowPersistentDamage) return false;
    return Item == this->CurrentInventoryItem;
}

FVector2D AGlobalPlayerController::GetLookSensitivity() {
    return this->LookSensitivity;
}

AItemActor* AGlobalPlayerController::GetManageItemTarget() {
    return this->CurrentInventoryItem;
}
