#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"

#include "ItemActor.h"

#include "InventorySlotComponent.generated.h"

class UInventoryComponent;

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TESTBED_API UInventorySlotComponent : public USphereComponent {
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<AActor> HighlightType;

	/** What type of inventory slot this is. */
	UPROPERTY(EditDefaultsOnly, Category="Inventory Slot")
	FString Type = "Normal"; // UENUM: Normal, Equip, Restricted
	/** An identifier for the slot that is unique within the actor. */
	UPROPERTY(EditDefaultsOnly, Category="Inventory Slot")
	FString SlotKey;
	/** What item "required slot types" this slot allows. */
	UPROPERTY(EditDefaultsOnly, Category="Inventory Slot")
	TArray<FString> AllowedItemTypes;
	UPROPERTY(EditDefaultsOnly, Category="Inventory Slot")
	FString AnimationProxyAnchor;

	// Refs.
	UInventoryComponent* ParentInventory;

	// State.
	AItemActor* Item;
	AActor* Highlight;

public:	
	UInventorySlotComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick Type, FActorComponentTickFunction* Self) override;

public:
	void BindToInventory(UInventoryComponent* TargetInventory);

	void SetItem(AItemActor* NewItem);
	bool CanHoldItem(AItemActor* Item);

	virtual FString GetKey();

	UFUNCTION(BlueprintCallable)
	AItemActor* GetItem() { return this->Item; }
	FString GetType() { return this->Type; }
	FString GetAnimationProxyAnchor() { return this->AnimationProxyAnchor; }

};
