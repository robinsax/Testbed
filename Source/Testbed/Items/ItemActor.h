#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Texture2D.h"

#include "../Lerp.h"
#include "../Animation/ProceduralAnimationComponent.h"
#include "InventoryComponent.h"

#include "ItemActor.generated.h"

class ABaseRobot;

USTRUCT(BlueprintType)
struct FItemEquipModifiers {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Modifiers")
	float RootYaw = 0.0f;

};

UCLASS()
class AItemActor : public AActor, public IInventoryOwner {
	GENERATED_BODY()

private:
	/** Determines whether inventory slot can contain. */
	UPROPERTY(EditDefaultsOnly, Category="Item")
	FString RequiredSlotType = "SET ME";
	/** Modifiers applied when this is the equipped item. */
	UPROPERTY(EditDefaultsOnly, Category="Item")
	FItemEquipModifiers EquipModifiers;
	UPROPERTY(EditDefaultsOnly, Category="Item")
	FRotator InSlotRotation;
	UPROPERTY(EditDefaultsOnly, Category="Item")
	UTexture2D* ItemIcon;
	UPROPERTY(EditDefaultsOnly, Category="Item")
	bool Useable;

	UPROPERTY(EditDefaultsOnly, Category="Lerp")
	FLerpedRotator RotationLerp;

	// Components.
	UPrimitiveComponent* Collider;
	UStaticMeshComponent* Mesh;
	UProceduralAnimationComponent* ProceduralAnimator;

	// State.
	bool InventoryCollisionEnabled;

	UPROPERTY(Replicated)
	bool Takeable = true;

protected:
	UInventoryComponent* Inventory;

public:
	AItemActor();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	void ServerSetTakeable(bool Update);

	UFUNCTION(BlueprintNativeEvent) // TODO: Blueprint pre-use checks.
	void ServerUseOnRobot(ABaseRobot* Target);

	/** Toggle between world and inventory collision modes. */
	void SetInventoryCollision(bool Enabled);
	virtual void SetTargetRotation(FRotator TargetRotation);

	ABaseRobot* GetRobotEquippedTo();

	UFUNCTION(BlueprintCallable)
	FItemEquipModifiers GetEquipModifiers() { return this->EquipModifiers; }
	UFUNCTION(BlueprintCallable)
	virtual UInventoryComponent* GetInventory() { return this->Inventory; }
	FString GetRequiredSlotType();
	UProceduralAnimationComponent* GetProceduralAnimator() { return this->ProceduralAnimator; }
	FRotator GetInSlotRotation() { return this->InSlotRotation; }

	bool IsUseable() { return this->Useable; }
	UFUNCTION(BlueprintCallable)
	bool IsTakeable() { return this->Takeable; }

};
