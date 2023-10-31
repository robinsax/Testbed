#include "BaseRobot.h"

#include "Net/UnrealNetwork.h"

#include "../Util.h"
#include "../KeyableComponent.h"
#include "../Meta/GlobalPlayerController.h"
#include "../Items/AttachmentItem.h"
#include "../Items/GunItem.h"

ABaseRobot::ABaseRobot() {
	PrimaryActorTick.bCanEverTick = true;
}

void ABaseRobot::BeginPlay() {
	Super::BeginPlay();

	this->CameraController = this->GetComponentByClass<URobotCameraController>();
	this->Inventory = this->GetComponentByClass<UInventoryComponent>();
	this->ProceduralAnimator = this->GetComponentByClass<UProceduralAnimationComponent>();
	this->Eye = UAnchorComponent::FindOne(this, TEXT("Eye"));

	this->GetComponents(this->Parts);
	for (int Idx = 0; Idx < this->Parts.Num(); Idx++) {
		this->Parts[Idx]->BindToRobot(this);
	}

	this->BaseAnimInstance = Cast<UBaseRobotAnimInstance>(this->GetMesh()->GetAnimInstance());
	this->BaseAnimInstance->BindToRobot(this);

	this->Stamina = this->MaxStamina / 2.0f;
	this->Energy = this->MaxEnergy;
}

void ABaseRobot::SetupPlayerInputComponent(UInputComponent* Input) {
    Super::SetupPlayerInputComponent(Input);

	Input->BindAxis("MoveForward", this, &ABaseRobot::InputMoveForward);
	Input->BindAxis("MoveRight", this, &ABaseRobot::InputMoveRight);

	Input->BindAxis("LookUp", this, &ABaseRobot::InputLookUp);
	Input->BindAxis("LookRight", this, &ABaseRobot::InputLookRight);

    Input->BindAction("Zoom", IE_Pressed, this, &ABaseRobot::InputStartZoom);
    Input->BindAction("Zoom", IE_Released, this, &ABaseRobot::InputEndZoom);

    Input->BindAction("Interact", IE_Pressed, this, &ABaseRobot::InputInteract);

    Input->BindAction("ToggleAttachments", IE_Pressed, this, &ABaseRobot::InputToggleAttachments);
}

void ABaseRobot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABaseRobot, RepWorldLookLocation, COND_None);
	DOREPLIFETIME_CONDITION(ABaseRobot, Energy, COND_None);
	DOREPLIFETIME_CONDITION(ABaseRobot, Stamina, COND_None);
}

void ABaseRobot::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if (this->GetLocalRole() == ROLE_Authority) {
		for (int Idx = 0; Idx < this->Parts.Num(); Idx++) {
			if (this->Parts[Idx]->IsProxy()) continue;

			this->Energy -= this->Parts[Idx]->GetCurrentEnergyDrain() * DeltaTime;
		}

		if (!this->UsingStamina && this->Stamina < this->MaxStamina) {
			if (this->Energy > 0.0f) {
				this->Stamina += this->StaminaRegain * DeltaTime;
				this->Energy -= this->StaminaEnergyCost * DeltaTime;
			}
		}
		this->UsingStamina = false;
	}
	else if (this->GetLocalRole() == ROLE_AutonomousProxy) { // TODO: Update check for server control.
		AGlobalPlayerController* PlayerController = Cast<AGlobalPlayerController>(this->GetController());
		if (IsValid(PlayerController)) {
			this->ControlTick(PlayerController, DeltaTime);
		}
	}
	else if (this->GetLocalRole() == ROLE_SimulatedProxy) {
		this->SimulatedTick(DeltaTime);
	}

	this->BaseAnimInstance->Tick(DeltaTime);
}

void ABaseRobot::SimulatedTick(float DeltaTime) {
	this->WorldLookLocation = this->RepWorldLookLocation;
}

void ABaseRobot::ControlTick(AGlobalPlayerController* PlayerController, float DeltaTime) {
	if (!this->BaseAnimInstance->HasActiveAnimationAtLevel(PARL_Base)) this->PlayBaseAnimation();

	if (PlayerController->GetMode() == EPlayerControlMode::Normal) {
        if (!this->MovementInput.IsZero()) {
            FVector Move = FVector(this->MovementInput.X, this->MovementInput.Y, 0.0).GetSafeNormal();

			Move = this->ControlProcessMove(Move);

            this->AddMovementInput(this->GetActorRotation().RotateVector(Move));
        }

        if (!this->LookInput.IsZero()) {
            FVector2D LookSensitivity = PlayerController->GetLookSensitivity();
            this->CameraController->UpdateCameraPitch(this->LookInput.Y * LookSensitivity.Y * DeltaTime);
            this->AddControllerYawInput(this->LookInput.X * LookSensitivity.X * DeltaTime);
        }
	}

	FScreenRaycastResult CenterScreenHit = PlayerController->ScreenRaycast(FVector2D(0.5f, 0.5f), -1.0f);

	this->WorldLookLocation = CenterScreenHit.EndLocation;
	this->ServerSetWorldLookLocation(this->WorldLookLocation); // TODO: Netspam.

	if (this->HadInteractInput && IsValid(CenterScreenHit.Actor)) {
		AItemActor* Item = Cast<AItemActor>(CenterScreenHit.Actor);
		
		if (IsValid(Item)) this->TakeItem(Item);
	}

	this->HadInteractInput = false;
}

FVector ABaseRobot::ControlProcessMove(FVector Move) {
	return Move;
}

void ABaseRobot::ServerSetWorldLookLocation_Implementation(FVector Update) {
    this->RepWorldLookLocation = Update;
    this->WorldLookLocation = Update;
}

AItemActor* ABaseRobot::GetEquippedItem() {
	UInventorySlotComponent* Slot = this->Inventory->GetEquipSlot();
	if (!IsValid(Slot)) return nullptr;

	return Slot->GetItem();
}

bool ABaseRobot::CanInteract() {
	return !this->BaseAnimInstance->HasActiveAnimationAtLevel(PARL_Interact);
}

float ABaseRobot::GetMaxHealth() {
	float MaxHealth = 0.0f;
	for (int Idx = 0; Idx < this->Parts.Num(); Idx++) {
		if (this->Parts[Idx]->IsProxy()) continue;
		MaxHealth += this->Parts[Idx]->GetMaxHealth();
	}

	return MaxHealth;
}

float ABaseRobot::GetHealth() {
	float Health = 0.0f;
	for (int Idx = 0; Idx < this->Parts.Num(); Idx++) {
		if (this->Parts[Idx]->IsProxy()) continue;
		Health += this->Parts[Idx]->GetHealth();
	}

	return Health;
}

int ABaseRobot::GetPersistentDamageCount() {
	int Count = 0;
	for (int Idx = 0; Idx < this->Parts.Num(); Idx++) {
		if (this->Parts[Idx]->IsProxy()) continue;
		if (this->Parts[Idx]->HasPersistentDamage()) Count++;
	}

	return Count;
}

URobotPartCollider* ABaseRobot::GetPartByKey(FString PartKey) {
	for (int Idx = 0; Idx < this->Parts.Num(); Idx++) {
		URobotPartCollider* Check = this->Parts[Idx];
		if (Check->GetPartKey().Equals(PartKey)) return Check;
	}

	return nullptr;
}

void ABaseRobot::ServerUpdateEnergy(float Amount) {
	this->Energy = FMath::Clamp(this->Energy + Amount, 0.0f, this->MaxEnergy);
}

void ABaseRobot::ServerHealPersistentDamage(URobotPartCollider* OnPart) {
	if (!OnPart->HasPersistentDamage()) {
		SANITY_ERR("ServerHealPersistentDamage: No persistent damage on part");
		return;
	}

	OnPart->ServerRegisterHealPersistentDamage();
	this->ServerOnHealthChanged();
}

void ABaseRobot::ServerHeal(float Amount) {
	if (Amount < 0.0f) {
		SANITY_ERR_D("ServerHeal: Amount=%.2f", Amount);
		return;
	}

	TArray<URobotPartCollider*> HealTargets;
	for (int Idx = 0; Idx < this->Parts.Num(); Idx++) {
		URobotPartCollider* Check = this->Parts[Idx];
		if (Check->IsProxy()) continue;
		if (Check->HasPersistentDamage()) continue;

		HealTargets.Push(Check);
	}
	if (HealTargets.Num() == 0) {
		SANITY_ERR("ServerHeal: No healable parts");
		return;
	}

	float PerPartHeal = Amount / HealTargets.Num();
	for (int Idx = 0; Idx < HealTargets.Num(); Idx++) {
		HealTargets[Idx]->ServerRegisterHeal(PerPartHeal);
	}
	this->ServerOnHealthChanged();
	return;
}

// Input.
void ABaseRobot::InputLookUp(float Value) {
    this->LookInput.Y = Value;
}

void ABaseRobot::InputLookRight(float Value) {
    this->LookInput.X = Value;
}

void ABaseRobot::InputMoveForward(float Value) {
    this->MovementInput.X = Value;
}

void ABaseRobot::InputMoveRight(float Value) {
    this->MovementInput.Y = Value;
}

void ABaseRobot::InputInteract() {
    this->HadInteractInput = true;
}

void ABaseRobot::InputStartZoom() {
    this->CameraController->SetZoom(true);
}

void ABaseRobot::InputEndZoom() {
    this->CameraController->SetZoom(false);
}

void ABaseRobot::InputToggleAttachments() {
	this->AttachmentToggle = !this->AttachmentToggle;

	this->ApplyToggleAttachments();
	this->ServerToggleAttachments(this->AttachmentToggle);
}

// Animation system.
void ABaseRobot::PlayBaseAnimation() {
	FString Type = TEXT("Idle");
	if (this->IsAiming()) Type = TEXT("Aim");

	FProceduralAnimationRequest Request = FProceduralAnimationRequest(Type, PARL_Base);
	AItemActor* EquippedItem = this->GetEquippedItem();
	if (IsValid(EquippedItem)) {
		Request.SetContextActor(TEXT("EquippedItem"), EquippedItem);
	}

	this->PlayAnimation(Request, CProceduralAnimationAction());
}

void ABaseRobot::ApplyAnimation(FProceduralAnimationRequest Request, CProceduralAnimationAction Action) {
	Request.Robot = this;

	FProceduralAnimation Anim;

	AItemActor* EquippedItem = this->GetEquippedItem();
	if (IsValid(EquippedItem)) {
		Anim = EquippedItem->GetProceduralAnimator()->TryGetProceduralAnimation(Request);
	}
	if (Anim.Keys.Num() < 2) { // TODO: Careful.
		Anim = this->ProceduralAnimator->TryGetProceduralAnimation(Request);
	}

	Anim.Request = Request;
	Anim.Action = Action;
	this->BaseAnimInstance->PushAnimation(Anim);
}

void ABaseRobot::PlayAnimation(FProceduralAnimationRequest Request, CProceduralAnimationAction Action) {
	this->ServerPlayAnimation(Request);
	this->ApplyAnimation(Request, Action);
}

void ABaseRobot::ServerPlayAnimation_Implementation(FProceduralAnimationRequest Request) {
	this->ClientPlayAnimation(Request);
	
	// Note there is no associated action here since that mutation will be replicated separately.
	this->ApplyAnimation(Request, CProceduralAnimationAction());
}

void ABaseRobot::ClientPlayAnimation_Implementation(FProceduralAnimationRequest Request) {
	if (this->GetLocalRole() != ROLE_SimulatedProxy) return;

	if (!IsValid(this)) return; // ???
	// Note there is no associated action here since that mutation will be replicated separately.
	this->ApplyAnimation(Request, CProceduralAnimationAction());
}

// Health system.
void ABaseRobot::ServerOnHealthChanged() {
	for (int Idx = 0; Idx < this->Parts.Num(); Idx++) {
		URobotPartCollider* Part = this->Parts[Idx];
		if (Part->IsProxy()) continue;

		this->ClientUpdateHealth(Part->GetPartKey(), Part->GetHealth(), Part->HasPersistentDamage());
	}
}

void ABaseRobot::ServerRegisterDamage(URobotPartCollider* PrimaryPart, FDamageProfile DamageProfile) {
	if (!ASSERT_PTR_SANITY(PrimaryPart, "ServerRegisterDamage: Part")) return;

	float Overflow = PrimaryPart->ServerRegisterDamageAsPrimary(DamageProfile);
	if (Overflow > 0.0f) {
		TArray<URobotPartCollider*> OverflowParts;
		for (int Idx = 0; Idx < this->Parts.Num(); Idx++) {
			URobotPartCollider* Check = this->Parts[Idx];
			if (Check->IsProxy()) continue;
			if (Check != PrimaryPart && Check->GetHealth() > 0.0f) {
				OverflowParts.Push(Check);
			}
		}

		float OverflowPerPart = Overflow / OverflowParts.Num();
		for (int Idx = 0; Idx < OverflowParts.Num(); Idx++) {
			URobotPartCollider* Part = OverflowParts[Idx];

			Part->ServerRegisterDamageAsOverflow(OverflowPerPart);
		}
	}

	this->ServerOnHealthChanged();
}

void ABaseRobot::ClientUpdateHealth_Implementation(const FString& PartKey, float NewPartHealth, bool HasPersistentDamage) {
	URobotPartCollider* Part = this->GetPartByKey(PartKey);
	if (!ASSERT_PTR_SANITY(Part, "ClientUpdateHealth: Part")) return;

	Part->ClientUpdateHealth(NewPartHealth, HasPersistentDamage);
}

// Toggle attachments.
void ABaseRobot::ApplyToggleAttachments() {
	TArray<AAttachmentItem*> Attachments = this->Inventory->GetItemsOfType<AAttachmentItem>();

	for (int Idx = 0; Idx < Attachments.Num(); Idx++) {
		Attachments[Idx]->ToggleAttachmentState(this->AttachmentToggle);
	}
}

void ABaseRobot::ServerToggleAttachments_Implementation(bool State) { 
	this->AttachmentToggle = State;
	this->ApplyToggleAttachments();
	this->ClientToggleAttachments(State);
}

void ABaseRobot::ClientToggleAttachments_Implementation(bool State) {
	if (this->GetLocalRole() != ROLE_SimulatedProxy) return;

	this->AttachmentToggle = State;
	this->ApplyToggleAttachments();
}

// UseItem.
void ABaseRobot::UseItem() {
	if (!this->CanInteract()) return;

	AItemActor* Item = this->GetEquippedItem();
	if (!IsValid(Item) || !Item->IsUseable()) return;

	FProceduralAnimationRequest Request = FProceduralAnimationRequest(TEXT("UseItem"), PARL_Interact);
	Request.SetContextActor(TEXT("Item"), Item);

	CProceduralAnimationAction Action;
	Action.BindUObject(this, &ABaseRobot::AnimationApplyUseItem);

	this->PlayAnimation(Request, Action);
}

void ABaseRobot::AnimationApplyUseItem(FProceduralAnimation* Animation) {
	AItemActor* Item = Cast<AItemActor>(Animation->Request.GetContextActor(TEXT("Item")));
	if (!ASSERT_PTR_SANITY(Item, "AnimationApplyUseItem: Item")) return;

	this->ServerUseItem();
}

void ABaseRobot::ServerUseItem_Implementation() {
	AItemActor* Item = this->GetEquippedItem();
	if (!ASSERT_PTR_SANITY(Item, "ServerUseItem: Item")) return;
	if (!Item->IsUseable()) {
		SANITY_ERR("ServerUseItem: Item unuseable");
		return;
	}

	Item->ServerUseOnRobot(this);
}

// TakeItem.
void ABaseRobot::TakeItem(AItemActor* Item) {
	if (!this->CanInteract()) return;
	UInventorySlotComponent* Slot = this->Inventory->GetBestSlotForItem(Item, false);
	if (!IsValid(Slot)) return;

	this->TakeItemToSlot(Item, Slot);
}

void ABaseRobot::TakeItemToSlot(AItemActor* Item, UInventorySlotComponent* Slot) {
	if (!this->CanInteract()) return;

	FProceduralAnimationRequest Request = FProceduralAnimationRequest(TEXT("TakeItem"), PARL_Interact);
	Request.SetContextActor(TEXT("Item"), Item);
	Request.SetContextComponent(TEXT("ToSlot"), Slot);

	CProceduralAnimationAction Action;
	Action.BindUObject(this, &ABaseRobot::AnimationApplyTakeItem);

	this->PlayAnimation(Request, Action);
}

void ABaseRobot::ApplyTakeItem(AItemActor* Item, UInventorySlotComponent* Slot) {
	Item->SetInventoryCollision(true);
	Slot->SetItem(Item);
}

void ABaseRobot::AnimationApplyTakeItem(FProceduralAnimation* Animation) {
	AItemActor* Item = Cast<AItemActor>(Animation->Request.GetContextActor(TEXT("Item")));
	if (!ASSERT_PTR_SANITY(Item, "AnimationApplyTakeItem: Item")) return;

	// TODO: Check whether taken already.

	UInventorySlotComponent* Slot = Cast<UInventorySlotComponent>(Animation->Request.GetContextComponent(this, TEXT("ToSlot")));
	if (!ASSERT_PTR_SANITY(Slot, "AnimationApplyTakeItem: Slot")) return;
	
	// TODO: Check whether slot still available.

	this->ApplyTakeItem(Item, Slot);
	this->ServerApplyTakeItem(Item, Slot->GetKey());
	this->PlayBaseAnimation();
}

void ABaseRobot::ServerApplyTakeItem_Implementation(AItemActor* Item, const FString& SlotKey) {
	if (!ASSERT_PTR_SANITY(Item, "ServerApplyTakeItem: Item")) return;

	UInventorySlotComponent* Slot = this->Inventory->GetSlotByKey(SlotKey);
	if (!ASSERT_PTR_SANITY(Slot, "ServerApplyTakeItem: Slot")) return;

	this->ApplyTakeItem(Item, Slot);
	this->ClientApplyTakeItem(Item, SlotKey);
}

void ABaseRobot::ClientApplyTakeItem_Implementation(AItemActor* Item, const FString& SlotKey) {
	if (this->GetLocalRole() != ROLE_SimulatedProxy) return;

	if (!ASSERT_PTR_SANITY(Item, "ClientApplyTakeItem: Item")) return;

	UInventorySlotComponent* Slot = this->Inventory->GetSlotByKey(SlotKey);
	if (!ASSERT_PTR_SANITY(Slot, "ClientApplyTakeItem: Slot")) return;

	this->ApplyTakeItem(Item, Slot);
}

// DropItem.
void ABaseRobot::DropItem(AItemActor* Item) {
	if (!this->CanInteract()) return;

	UInventorySlotComponent* Slot = this->Inventory->GetSlotContainingItem(Item);
	if (!IsValid(Slot)) return;

	FProceduralAnimationRequest Request = FProceduralAnimationRequest(TEXT("DropItem"), PARL_Interact);
	Request.SetContextActor(TEXT("Item"), Item);
	Request.SetContextComponent(TEXT("FromSlot"), Slot);

	CProceduralAnimationAction Action;
	Action.BindUObject(this, &ABaseRobot::AnimationApplyDropItem);

	this->PlayAnimation(Request, Action);
}

void ABaseRobot::ApplyDropItem(AItemActor* Item) {
	UInventorySlotComponent* Slot = this->Inventory->GetSlotContainingItem(Item);
	if (!ASSERT_PTR_SANITY(Slot, "ApplyDropItem: Slot")) return;

	Slot->SetItem(nullptr);
	Item->SetInventoryCollision(false);
}

void ABaseRobot::AnimationApplyDropItem(FProceduralAnimation* Animation) {
	AItemActor* Item = Cast<AItemActor>(Animation->Request.GetContextActor(TEXT("Item")));

	// TODO: Check still valid.

	this->ServerDropItem(Item);
	this->PlayBaseAnimation();
}

void ABaseRobot::ServerDropItem_Implementation(AItemActor* Item) {
	this->ClientDropItem(Item);
	this->ApplyDropItem(Item);
}

void ABaseRobot::ClientDropItem_Implementation(AItemActor* Item) {
	if (this->GetLocalRole() == ROLE_Authority) return;
	
	this->ApplyDropItem(Item);
}

// MoveItem.
void ABaseRobot::MoveItem(AItemActor* Item, UInventorySlotComponent* NewSlot) {
	if (!this->CanInteract()) return;

	UInventorySlotComponent* FromSlot = this->Inventory->GetSlotContainingItem(Item);
	if (!IsValid(FromSlot)) return;

	FProceduralAnimationRequest Request = FProceduralAnimationRequest(TEXT("MoveItem"), PARL_Interact);
	Request.SetContextActor(TEXT("Item"), Item);
	Request.SetContextComponent(TEXT("ToSlot"), NewSlot);
	Request.SetContextComponent(TEXT("FromSlot"), FromSlot);

	CProceduralAnimationAction Action;
	Action.BindUObject(this, &ABaseRobot::AnimationApplyMoveItem);

	this->PlayAnimation(Request, Action);
}

void ABaseRobot::AnimationApplyMoveItem(FProceduralAnimation* Animation) {
	AItemActor* Item = Cast<AItemActor>(Animation->Request.GetContextActor(TEXT("Item")));
	if (!ASSERT_PTR_SANITY(Item, "AnimationApplyMoveItem: Item")) return;

	UInventorySlotComponent* ToSlot = Cast<UInventorySlotComponent>(Animation->Request.GetContextComponent(this, TEXT("ToSlot")));
	if (!ASSERT_PTR_SANITY(ToSlot, "AnimationApplyMoveItem: ToSlot")) return;

	this->ServerApplyMoveItem(Item, ToSlot->GetKey());
	this->ApplyMoveItem(Item, ToSlot);
	this->PlayBaseAnimation();
}

void ABaseRobot::ApplyMoveItem(AItemActor* Item, UInventorySlotComponent* NewSlot) {
	UInventorySlotComponent* PrevSlot = this->Inventory->GetSlotContainingItem(Item);
	if (!ASSERT_PTR_SANITY(PrevSlot, "ApplyMoveItem: PrevSlot")) return;

	PrevSlot->SetItem(nullptr);
	NewSlot->SetItem(Item);
}

void ABaseRobot::ServerApplyMoveItem_Implementation(AItemActor* Item, const FString& SlotKey) {
	if (!ASSERT_PTR_SANITY(Item, "ServerApplyMoveItem: Item")) return;

	UInventorySlotComponent* Slot = this->Inventory->GetSlotByKey(SlotKey);
	if (!ASSERT_PTR_SANITY(Slot, "ServerApplyMoveItem: Slot")) return;

	this->ApplyMoveItem(Item, Slot);
	this->ClientApplyMoveItem(Item, SlotKey);
}

void ABaseRobot::ClientApplyMoveItem_Implementation(AItemActor* Item, const FString& SlotKey) {
	if (this->GetLocalRole() != ROLE_SimulatedProxy) return;

	if (!ASSERT_PTR_SANITY(Item, "ClientApplyMoveItem: Item")) return;

	UInventorySlotComponent* Slot = this->Inventory->GetSlotByKey(SlotKey);
	if (!ASSERT_PTR_SANITY(Slot, "ClientApplyMoveItem: Slot")) return;

	this->ApplyMoveItem(Item, Slot);
}

// Fire.
void ABaseRobot::ServerFire_Implementation() {
	AGunItem* Gun = Cast<AGunItem>(this->GetEquippedItem());
	if (!ASSERT_PTR_SANITY(Gun, "ServerFire: Gun")) return;

	Gun->ServerFire();
}
