#include "InventoryComponent.h"

#include "../Util.h"
#include "../Robots/BaseRobot.h"
#include "AttachmentItem.h"
#include "ItemActor.h"
#include "InventorySlotComponent.h"
#include "MagazineItem.h"

void UInventoryComponent::BeginPlay() {
	Super::BeginPlay();

	this->RefreshSlots();
	for (int Idx = 0; Idx < this->Slots.Num(); Idx++) {
		this->Slots[Idx]->BindToInventory(this);
	}
}

void UInventoryComponent::RefreshSlots() {
	this->EquipSlot = nullptr;

	TArray<AActor*> CheckActors;
	this->GetOwner()->GetAttachedActors(CheckActors);
	CheckActors.Push(this->GetOwner());

	this->Slots = TArray<UInventorySlotComponent*>();
	for (int Idx = 0; Idx < CheckActors.Num(); Idx++) {
		TArray<UInventorySlotComponent*> SlotsHere;
		CheckActors[Idx]->GetComponents(SlotsHere);

		this->Slots.Append(SlotsHere);
	}

	for (int Idx = 0; Idx < this->Slots.Num(); Idx++) {
		UInventorySlotComponent* Check = this->Slots[Idx];
		if (!Check->GetType().Equals(TEXT("Equip"))) continue;

		SANITY_WARN_IF(IsValid(this->EquipSlot), "Multiple equip slots");
		this->EquipSlot = Check;

		#ifndef SANITY_CHECKS
		break;
		#endif
	}
}

bool UInventoryComponent::CanTakeItem(AItemActor* Item) {
	return IsValid(this->GetBestSlotForItem(Item, false));
}

bool UInventoryComponent::ContainsItem(AItemActor* Item) {
	return IsValid(this->GetSlotContainingItem(Item));
}

UInventoryComponent* UInventoryComponent::GetParentInventory() {
	AItemActor* OwnerItem = Cast<AItemActor>(this->GetOwner());
	if (!IsValid(OwnerItem)) return nullptr; // We're on a robot.

	AActor* AttachParent = OwnerItem->GetAttachParentActor();
	if (!IsValid(AttachParent)) return nullptr;

	// TODO: Suggests an interface is needed.
	AItemActor* ParentItem = Cast<AItemActor>(AttachParent);
	if (IsValid(ParentItem)) return ParentItem->GetInventory();

	ABaseRobot* ParentRobot = Cast<ABaseRobot>(AttachParent);
	if (IsValid(ParentRobot)) return ParentRobot->GetInventory();

	SANITY_WARN("GetParentInventory: Unknown attach parent actor type");
	return nullptr;
}

UInventorySlotComponent* UInventoryComponent::GetSlotByKey(FString SlotKey) {
	for (int Idx = 0; Idx < this->Slots.Num(); Idx++) {
		UInventorySlotComponent* Slot = this->Slots[Idx];

		if (Slot->GetKey().Equals(SlotKey)) return Slot;
	}

	return nullptr;
}

UInventorySlotComponent* UInventoryComponent::GetSlotContainingItem(AItemActor* Item) {
	for (int I = 0; I < this->Slots.Num(); I++) {
		UInventorySlotComponent* Slot = this->Slots[I];
		
		if (Slot->GetItem() == Item) return Slot;
	}

	return nullptr;
}

template<typename T>
TArray<T*> UInventoryComponent::GetItemsOfType() {
	TArray<T*> Items;
	
	for (int Idx = 0; Idx < this->Slots.Num(); Idx++) {
		T* Item = Cast<T>(this->Slots[Idx]->GetItem());
		if (!IsValid(Item)) continue;

		Items.Push(Item);
	}

	return Items;
}
template TArray<class AAttachmentItem*> UInventoryComponent::GetItemsOfType<AAttachmentItem>();

// TODO: Smarter.
UInventorySlotComponent* UInventoryComponent::GetBestSlotForItem(AItemActor* Item, bool AllowRestricted) {
	UInventorySlotComponent* WorstCase = nullptr;
	for (int I = 0; I < this->Slots.Num(); I++) {
		UInventorySlotComponent* Slot = this->Slots[I];

		if (!AllowRestricted && Slot->GetType().Equals(TEXT("Restricted"))) continue;
		if (Slot->CanHoldItem(Item)) {
			if (Slot == this->GetEquipSlot()) {
				WorstCase = Slot;
			}
			else {
				return Slot;
			}
		}
	}

	return nullptr;
}

// TODO: Smarter.
UInventorySlotComponent* UInventoryComponent::GetBestSlotContainingMagazine(FString MagazineType) {
	for (int I = 0; I < this->Slots.Num(); I++) {
		AMagazineItem* Item = Cast<AMagazineItem>(this->Slots[I]->GetItem());
		if (!IsValid(Item)) continue;

		if (Item->GetMagazineType().Equals(MagazineType)) return this->Slots[I];
	}

	return nullptr;
}
