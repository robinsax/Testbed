#include "HumanoidRobot.h"

#include "Net/UnrealNetwork.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "../Util.h"
#include "../Meta/GlobalPlayerController.h"
#include "../Items/GunItem.h"

// TODO: ONLY ALLOW AIMING IF ITEM IS AIMABLE

void AHumanoidRobot::BeginPlay() {
    Super::BeginPlay();

    this->AnimInstance = Cast<UHumanoidRobotAnimInstance>(this->GetMesh()->GetAnimInstance());

    this->BaseCapsuleHalfHeight = this->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    this->BaseMeshVerticalOffset = this->GetMesh()->GetRelativeLocation().Z;

    this->CrouchLerp.SetBaseValue(0.0f);

    this->MovementState = HRMS_Walking;
    
    this->WorldHUDWidget = this->GetComponentByClass<UWidgetComponent>();
    TArray<USceneComponent*> HUDComponents = this->WorldHUDWidget->GetAttachChildren();
    HUDComponents.Push(this->WorldHUDWidget);
    for (int Idx = 0; Idx < HUDComponents.Num(); Idx++) {
        HUDComponents[Idx]->SetVisibility(false);
    }

	Cast<URobotHUDWidget>(this->WorldHUDWidget->GetUserWidgetObject())->Robot = this;

    for (int Idx = 0; Idx < 5; Idx++) {
        this->QuickSlots.Push(nullptr);
    }
}

void AHumanoidRobot::SetupPlayerInputComponent(UInputComponent* Input) {
    Super::SetupPlayerInputComponent(Input);

	Input->BindAxis("LeanRight", this, &AHumanoidRobot::InputLeanRight);

    Input->BindAction("Jump", IE_Pressed, this, &AHumanoidRobot::InputStartJump);

    Input->BindAction("Reload", IE_Pressed, this, &AHumanoidRobot::InputReload);

    Input->BindAction("Crouch", IE_Pressed, this, &AHumanoidRobot::InputStartCrouch);
    Input->BindAction("Crouch", IE_Released, this, &AHumanoidRobot::InputEndCrouch);

    Input->BindAction("Sprint", IE_Pressed, this, &AHumanoidRobot::InputStartSprint);
    Input->BindAction("Sprint", IE_Released, this, &AHumanoidRobot::InputEndSprint);
    
    Input->BindAction("CheckStatus", IE_Pressed, this, &AHumanoidRobot::InputStartCheckStatus);
    Input->BindAction("CheckStatus", IE_Released, this, &AHumanoidRobot::InputEndCheckStatus);
    
    Input->BindAction("QuickSlot1", IE_Pressed, this, &AHumanoidRobot::InputQuickSlot1);
    Input->BindAction("QuickSlot2", IE_Released, this, &AHumanoidRobot::InputQuickSlot2);
    Input->BindAction("QuickSlot3", IE_Released, this, &AHumanoidRobot::InputQuickSlot3);
    Input->BindAction("QuickSlot4", IE_Released, this, &AHumanoidRobot::InputQuickSlot4);
    Input->BindAction("QuickSlot5", IE_Released, this, &AHumanoidRobot::InputQuickSlot5);
}

void AHumanoidRobot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AHumanoidRobot, RepMovementState, COND_None);
	DOREPLIFETIME_CONDITION(AHumanoidRobot, RepLean, COND_None);
	DOREPLIFETIME_CONDITION(AHumanoidRobot, RepFiring, COND_None);
	DOREPLIFETIME_CONDITION(AHumanoidRobot, RepCheckingStatus, COND_None);
}

void AHumanoidRobot::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    if (this->GetLocalRole() == ROLE_Authority) {
        if (this->Firing) {
            AGunItem* Gun = Cast<AGunItem>(this->GetEquippedItem());
            if (IsValid(Gun)) {
                Gun->ServerFire();
            }
        }

        if (this->MovementState == HRMS_Sprinting) {
            this->Stamina -= this->SprintStaminaCost * DeltaTime;
            this->UsingStamina = true;
        }
    }

    // TODO: Local only and better.
    TArray<USceneComponent*> HUDComponents = this->WorldHUDWidget->GetAttachChildren();
    HUDComponents.Push(this->WorldHUDWidget);
    for (int Idx = 0; Idx < HUDComponents.Num(); Idx++) {
        HUDComponents[Idx]->SetVisibility(this->CheckingStatus);
    }

    if (this->CheckingStatus) {
        FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(this->WorldHUDWidget->GetComponentLocation(), this->GetEye()->GetComponentLocation());
        Rotation.Roll = 0.0f;
        this->WorldHUDWidget->SetWorldRotation(Rotation);
    }

    this->CrouchLerp.SetTargetValue(this->IsCrouching() ? 1.0f : 0.0f);
    float CrouchCoef = this->CrouchLerp.Update(DeltaTime);

    FVector MeshOffset = this->GetMesh()->GetRelativeLocation();
    MeshOffset.Z = this->BaseMeshVerticalOffset - (this->CrouchMeshVerticalOffset * CrouchCoef);
    this->GetMesh()->SetRelativeLocation(MeshOffset);

    float CapsuleHalfHeight = this->BaseCapsuleHalfHeight - (this->BaseCapsuleHalfHeight * this->CrouchHeightModifier * CrouchCoef);
    this->GetCapsuleComponent()->SetCapsuleHalfHeight(CapsuleHalfHeight, true);
}

void AHumanoidRobot::ControlTick(AGlobalPlayerController* PlayerController, float DeltaTime) {
    Super::ControlTick(PlayerController, DeltaTime);

    if (this->IsSprinting() && this->Stamina <= 0.0f) {
        this->MovementState = HRMS_Walking;
        this->ServerSetMovementState(HRMS_Walking);
    }
    this->CameraController->SetSprint(this->IsSprinting());

    if (!this->Firing && (this->MovementState & HRMS_Hipfire) > 0) {
        this->MovementState ^= HRMS_Hipfire;
    }

    bool WasInInventory = this->MovementState == HRMS_InInventory;
    if (PlayerController->GetMode() == EPlayerControlMode::Inventory) {
        if (!WasInInventory) {
            this->MovementState = HRMS_InInventory;
            this->ServerSetMovementState(HRMS_InInventory);
        }
        
        this->AddControllerYawInput(this->MovementInput.Y * 90.0f * DeltaTime);
    }
    else {
        if (WasInInventory) {
            this->MovementState = HRMS_Walking;
            this->ServerSetMovementState(HRMS_Walking);
        }
        
        this->Lean = this->LeanInput * this->MaxLeanDegrees;
        this->ServerSetLean(this->Lean); // TODO: Netspam.
        this->CameraController->SetLean(this->LeanInput);
    }

    if (this->MovementState != this->PrevMovementState) {
        this->PlayBaseAnimation();
    }
    this->PrevMovementState = this->MovementState;
}

FVector AHumanoidRobot::ControlProcessMove(FVector Move) { // TODO: Can use to validate movement on server later.
    float MoveAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Move, FVector(1.0, 0.0, 0.0))));
    
    bool IsStrafe = MoveAngle > this->StrafeAngle;
    
    if (IsStrafe) {
        Move *= this->StrafeSpeedModifier;
    }
    if (IsStrafe || this->MovementState != HRMS_Sprinting) {
        Move *= this->WalkSpeedModifier;
    }
    if (this->IsCrouching()) {
        Move *= this->CrouchSpeedModifier;
    }
    if (this->Energy <= 0.0f && this->Stamina <= 0.0f) {
        Move *= this->NoEnergySpeedModifier;
    }

    return Move;
}

void AHumanoidRobot::SimulatedTick(float DeltaTime) {
    Super::SimulatedTick(DeltaTime);

    this->MovementState = this->RepMovementState;
    this->Lean = this->RepLean;
    this->Firing = this->RepFiring;
    this->CheckingStatus = this->RepCheckingStatus;
}

void AHumanoidRobot::PlayBaseAnimation() {
    FProceduralAnimationRequest Request;
    Request.Level = PARL_Base;

    if (this->CheckingStatus) {
        Request.Type = TEXT("CheckStatus");
    }
    else if (this->MovementState == HRMS_InInventory) {
        Request.Type = TEXT("Inventory");
    }
    else if ((this->MovementState & HRMS_Hipfire) > 0) {
        Request.Type = TEXT("Hipfire");
    }
    else {
        Super::PlayBaseAnimation();
        return;
    }

    AItemActor* EquippedItem = this->GetEquippedItem();
    if (IsValid(EquippedItem)) {
        Request.SetContextActor(TEXT("EquippedItem"), EquippedItem);
    }
    
    this->PlayAnimation(Request, CProceduralAnimationAction());
    this->PlayAnimation(Request, CProceduralAnimationAction());
}

bool AHumanoidRobot::IsAiming() {
    return (this->MovementState & HRMS_Aiming) > 0;
}

bool AHumanoidRobot::IsFiring() {
    return this->Firing;
}

bool AHumanoidRobot::IsCrouching() {
    return (this->MovementState & HRMS_Crouching) > 0;
}

bool AHumanoidRobot::IsInInventory() {
    return this->MovementState == HRMS_InInventory;
}

bool AHumanoidRobot::IsSprinting() { // TODO: Doesn't work on server.
    return this->MovementState == HRMS_Sprinting && this->MovementInput.X > 0.0f;
}

// Input.
void AHumanoidRobot::InputStartCheckStatus() {
	this->CheckingStatus = true;
    this->ServerSetCheckingStatus(true);
    this->PlayBaseAnimation();
}

void AHumanoidRobot::InputEndCheckStatus() {
	this->CheckingStatus = false;
    this->ServerSetCheckingStatus(false);
    this->PlayBaseAnimation();
}

void AHumanoidRobot::InputLeanRight(float Value) {
    this->LeanInput = Value;
}

void AHumanoidRobot::InputStartSprint() {
    this->MovementState = HRMS_Sprinting;
    this->ServerSetMovementState(HRMS_Sprinting);
}

void AHumanoidRobot::InputEndSprint() {
    if (this->MovementState != HRMS_Sprinting) return;

    this->MovementState = HRMS_Walking;
    this->ServerSetMovementState(HRMS_Walking);
}

void AHumanoidRobot::InputStartCrouch() {
    int NewState = HRMS_Crouching;
    if (this->IsAiming()) NewState |= HRMS_Aiming;

    this->MovementState = NewState;
    this->ServerSetMovementState(NewState);
}

void AHumanoidRobot::InputEndCrouch() {
    int NewState = this->MovementState ^ HRMS_Crouching;

    this->MovementState = NewState;
    this->ServerSetMovementState(NewState);
}

void AHumanoidRobot::InputStartAim() {
    if (this->CheckingStatus) return;
    if (this->IsAiming()) return;

    int NewState = HRMS_Aiming;
    if (this->IsCrouching()) NewState |= HRMS_Crouching;

    this->MovementState = NewState;
    this->ServerSetMovementState(NewState);
}

void AHumanoidRobot::InputEndAim() {
    int NewState = this->MovementState ^ HRMS_Aiming;

    this->MovementState = NewState;
    this->ServerSetMovementState(NewState);
}

void AHumanoidRobot::InputStartFire() { // TODO: Rename.
    AItemActor* EquippedItem = this->GetEquippedItem();
    if (!IsValid(EquippedItem)) return;

    if (EquippedItem->IsUseable()) {
        if (this->IsInInventory()) return;

        this->UseItem();
        return;
    }

    AGunItem* Gun = Cast<AGunItem>(EquippedItem);
    if (!IsValid(Gun) || !Gun->CanFire()) return;
    if (this->CheckingStatus) return;

    if (!this->IsAiming()) {
        int NewState = HRMS_Hipfire;
        if (this->IsCrouching()) NewState |= HRMS_Crouching;

        this->MovementState = NewState;
        this->ServerSetMovementState(NewState);
    }

    this->Firing = true;
    this->ServerSetFiring(true);
}

void AHumanoidRobot::InputEndFire() {
    this->Firing = false;
    this->ServerSetFiring(false);
}

void AHumanoidRobot::InputQuickSlot(int Idx) {
    if (this->IsInInventory()) {
	    AGlobalPlayerController* PlayerController = AGlobalPlayerController::Get(this->GetWorld());
        AItemActor* TargetItem = PlayerController->GetManageItemTarget();
        if (!IsValid(TargetItem) || !this->Inventory->ContainsItem(TargetItem)) return;

        this->QuickSlots[Idx] = TargetItem;
    }
    else {
        AItemActor* TargetItem = this->QuickSlots[Idx];
        if (!IsValid(TargetItem)) return;

        UInventorySlotComponent* CurrentSlot = this->Inventory->GetSlotContainingItem(TargetItem);
        if (!IsValid(CurrentSlot)) return;

        UInventorySlotComponent* EquipSlot = this->Inventory->GetEquipSlot();
        if (EquipSlot == CurrentSlot) {
            UInventorySlotComponent* NewSlot = this->Inventory->GetBestSlotForItem(TargetItem, false);
            if (IsValid(NewSlot) && NewSlot != EquipSlot) {
                this->MoveItem(TargetItem, NewSlot);
            }

            return;
        }
        if (!EquipSlot->CanHoldItem(TargetItem)) return;

        this->MoveItem(TargetItem, EquipSlot);
    }
}

// Simple reps.
void AHumanoidRobot::ServerSetCheckingStatus_Implementation(bool Update) {
    this->RepCheckingStatus = Update;
    this->CheckingStatus = Update;
}

void AHumanoidRobot::ServerSetMovementState_Implementation(int Update) {
    this->RepMovementState = Update;
    this->MovementState = Update;
}

void AHumanoidRobot::ServerSetLean_Implementation(float Update) {
    this->RepLean = Update;
    this->Lean = Update;
}

void AHumanoidRobot::ServerSetFiring_Implementation(bool Update) {
    this->Firing = Update;
    this->RepFiring = Update;
}

// Reload.
void AHumanoidRobot::InputReload() { // TODO: Separate from input.
    AGunItem* Gun = Cast<AGunItem>(this->GetEquippedItem());
    if (!IsValid(Gun)) return;
    
    UInventorySlotComponent* MagazineSlot = this->Inventory->GetBestSlotContainingMagazine(Gun->GetRequiredMagazineType());
    if (!IsValid(MagazineSlot)) return;

    AMagazineItem* Magazine = Cast<AMagazineItem>(MagazineSlot->GetItem());

    FProceduralAnimationRequest Request = FProceduralAnimationRequest(TEXT("Reload"), PARL_Interact);
	Request.SetContextActor(TEXT("Magazine"), Magazine);
    Request.SetContextActor(TEXT("Gun"), Gun);
	Request.SetContextComponent(TEXT("MagazineSlot"), MagazineSlot);

	CProceduralAnimationAction Action;
	Action.BindUObject(this, &AHumanoidRobot::AnimationApplyReload);

	this->PlayAnimation(Request, Action);
}

void AHumanoidRobot::AnimationApplyReload(FProceduralAnimation* Animation) {
    AMagazineItem* Magazine = Cast<AMagazineItem>(Animation->Request.GetContextActor(TEXT("Magazine")));
    if (!ASSERT_PTR_SANITY(Magazine, "AnimationApplyReload: Magazine")) return;

    this->ServerApplyReload(Magazine);
    this->ApplyReload(Magazine);
}

void AHumanoidRobot::ApplyReload(AMagazineItem* Magazine) {
    AGunItem* Gun = Cast<AGunItem>(this->GetEquippedItem());
    if (!ASSERT_PTR_SANITY(Gun, "ApplyReload: Gun")) return;
    UInventorySlotComponent* FromSlot = this->Inventory->GetSlotContainingItem(Magazine);
    if (!ASSERT_PTR_SANITY(FromSlot, "ApplyReload: FromSlot")) return;

	FromSlot->SetItem(nullptr);
    Gun->LoadMagazine(Magazine);
}

void AHumanoidRobot::ServerApplyReload_Implementation(AMagazineItem* Magazine) {
    if (!this->Inventory->ContainsItem(Magazine)) return; // todo client error.

    this->ApplyReload(Magazine);
    this->ClientApplyReload(Magazine);
}

void AHumanoidRobot::ClientApplyReload_Implementation(AMagazineItem* Magazine) {
    this->ApplyReload(Magazine);
}

// Jump.
bool AHumanoidRobot::ApplyJump() {
    UCharacterMovementComponent* Movement = this->GetCharacterMovement();
    if (Movement->IsFalling() || this->IsCrouching() || this->Stamina <= this->JumpStaminaCost) return false;

    Movement->AddImpulse(FVector(0.0, 0.0, this->JumpStrength * 100.0));
    return true;
}

void AHumanoidRobot::ServerStartJump_Implementation() {
    if (this->ApplyJump()) this->Stamina -= this->JumpStaminaCost;
}

void AHumanoidRobot::InputStartJump() { // TODO: Separate from input.
    if (AGlobalPlayerController::GetMode(this->GetWorld()) != EPlayerControlMode::Normal) return;

    this->ApplyJump();
    this->ServerStartJump();
}
