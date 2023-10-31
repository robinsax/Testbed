#include "InventorySlotComponent.h"

#include "../Util.h"
#include "../KeyableComponent.h"
#include "../Meta/GlobalPlayerController.h"
#include "InventoryComponent.h"

UInventorySlotComponent::UInventorySlotComponent() {
	PrimaryComponentTick.bCanEverTick = true;
}

void UInventorySlotComponent::BindToInventory(UInventoryComponent* TargetInventory) {
	this->ParentInventory = TargetInventory;
}

void UInventorySlotComponent::TickComponent(float DeltaTime, ELevelTick Type_, FActorComponentTickFunction* Self) {
	Super::TickComponent(DeltaTime, Type_, Self);

	// TODO: Implement this in a better way. It's tricky to do in player controller because the set of slots is stateful.

	// Get top level actor's inventory.
	UInventoryComponent* Inventory = this->ParentInventory;
	while (IsValid(Inventory->GetParentInventory())) {
		Inventory = Inventory->GetParentInventory();
	}
	// Ensure we're within the local player. Guh.
	if (Inventory->GetOwner()->GetLocalRole() != ROLE_AutonomousProxy) return;

	AGlobalPlayerController* Controller = AGlobalPlayerController::Get(this->GetWorld());
	if (!ASSERT_PTR_SANITY(Controller, "InventorySlotComponent tick: Controller")) return;

	// Toggle existance of highlight.
	if (Controller->IsSlotDropTarget(this)) {
		if (!IsValid(this->Highlight)) {
			this->Highlight = this->GetWorld()->SpawnActor<AActor>(
				this->HighlightType,
				FVector(),
				FRotator(),
				FActorSpawnParameters()
			);
			this->Highlight->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}
	}
	else if (IsValid(this->Highlight)) {
		this->Highlight->Destroy();
		this->Highlight = nullptr;
	}
}

bool UInventorySlotComponent::CanHoldItem(AItemActor* TargetItem) {
	return !IsValid(this->Item) && this->AllowedItemTypes.Contains(TargetItem->GetRequiredSlotType());
}

void UInventorySlotComponent::SetItem(AItemActor* NewItem) {
	if (this->Item) {
		this->Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
	if (NewItem) {
		NewItem->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		NewItem->SetActorRelativeRotation(NewItem->GetInSlotRotation());
	}
	this->Item = NewItem;

	// Because game logic invokes this directly, we need to notify the parent inventory.
	this->ParentInventory->RefreshSlots();
}

FString UInventorySlotComponent::GetKey() {
	UInventoryComponent* ParentParentInventory = this->ParentInventory->GetParentInventory();
	if (!IsValid(ParentParentInventory)) return "Slot:" + this->SlotKey;

	// Invariant: we're a slot of an item's inventory.
	UInventorySlotComponent* ParentSlot = ParentParentInventory->GetSlotContainingItem(Cast<AItemActor>(this->GetOwner()));

	return ParentSlot->GetKey() + "__" + this->SlotKey;
}
