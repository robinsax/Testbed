#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "InventoryComponent.generated.h"

class AItemActor;
class UInventorySlotComponent;

UINTERFACE(MinimalAPI)
class UInventoryOwner : public UInterface {
	GENERATED_BODY()
};

class IInventoryOwner {
	GENERATED_BODY()

public:
	virtual UInventoryComponent* GetInventory() { return nullptr; };

};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UInventoryComponent : public UActorComponent {
	GENERATED_BODY()

	friend class UInventorySlotComponent;

private:
	// Components.
	TArray<UInventorySlotComponent*> Slots; // Stateful.
	UInventorySlotComponent* EquipSlot;

protected:
	virtual void BeginPlay() override;

public:
	void RefreshSlots();

	// Intentionally deciding against simple mutation methods as replication is a necessary
	// consideration and therefore actor-level logic must be aware of internals anyway.

	bool CanTakeItem(AItemActor* Item);
	bool ContainsItem(AItemActor* Item);

	/** Return the inventory of the actor this inventories' actor is attached to. */
	UInventoryComponent* GetParentInventory();

	template <typename T>
	TArray<T*> GetItemsOfType();

	UInventorySlotComponent* GetSlotByKey(FString SlotKey);
	UInventorySlotComponent* GetSlotContainingItem(AItemActor* Item);

	UInventorySlotComponent* GetBestSlotForItem(AItemActor* Item, bool AllowRestricted);
	UInventorySlotComponent* GetBestSlotContainingMagazine(FString MagazineType);

	UInventorySlotComponent* GetEquipSlot() { return this->EquipSlot; }
	TArray<UInventorySlotComponent*> GetSlots() { return this->Slots; }

};
