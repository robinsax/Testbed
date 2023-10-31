#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "../KeyableComponent.h"
#include "../Meta/RobotCameraController.h"
#include "../Items/InventoryComponent.h"
#include "../Items/InventorySlotComponent.h"
#include "../Items/ItemActor.h"
#include "../Animation/ProceduralAnimationComponent.h"
#include "../Animation/BaseRobotAnimInstance.h"
#include "RobotPartCollider.h"

#include "BaseRobot.generated.h"

class AGlobalPlayerController;

UCLASS()
class ABaseRobot : public ACharacter, public IInventoryOwner {
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category="Stats")
	int MaxEnergy = 300.0f;
	UPROPERTY(EditDefaultsOnly, Category="Stats")
	int MaxStamina = 300.0f;
	UPROPERTY(EditDefaultsOnly, Category="Stats")
	int StaminaRegain = 50.0f;
	UPROPERTY(EditDefaultsOnly, Category="Stats")
	int StaminaEnergyCost = 10.0f;

	// Components.
	UBaseRobotAnimInstance* BaseAnimInstance;
	UAnchorComponent* Eye;
	TArray<URobotPartCollider*> Parts;

	// State.
	UPROPERTY(Replicated)
	FVector RepWorldLookLocation;
	FVector WorldLookLocation;

	bool AttachmentToggle;

protected:
	// Input.
	bool HadInteractInput;
	FVector2D MovementInput;
	FVector2D LookInput;

	// Components.
	URobotCameraController* CameraController;
	UInventoryComponent* Inventory;
	UProceduralAnimationComponent* ProceduralAnimator;

	// State.
	// Note unlike other properties, autonomous proxies don't control these and read the reps.
	UPROPERTY(Replicated)
	float Energy;

	bool UsingStamina;
	UPROPERTY(Replicated)
	float Stamina;

public:
	ABaseRobot();

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* Input) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	virtual void ControlTick(AGlobalPlayerController* PlayerController, float DeltaTime);
	virtual void SimulatedTick(float DeltaTime);

private:
	void ApplyAnimation(FProceduralAnimationRequest Request, CProceduralAnimationAction Action);

protected:
	virtual bool CanInteract();
	virtual void PlayBaseAnimation();

	virtual FVector ControlProcessMove(FVector Move);

public:
	UFUNCTION(BlueprintCallable)
	void ServerUpdateEnergy(float Amount);
	UFUNCTION(BlueprintCallable)
	void ServerHeal(float Amount);
	UFUNCTION(BlueprintCallable)
	void ServerHealPersistentDamage(URobotPartCollider* OnPart);

	void PlayAnimation(FProceduralAnimationRequest Request, CProceduralAnimationAction Action);

	void TakeItem(AItemActor* Item);
	void TakeItemToSlot(AItemActor* Item, UInventorySlotComponent* Slot);
	void DropItem(AItemActor* Item);
	void MoveItem(AItemActor* Item, UInventorySlotComponent* NewSlot);
	void UseItem(); // TODO: Params how?

	UFUNCTION(BlueprintCallable)
	AItemActor* GetEquippedItem();

	URobotPartCollider* GetPartByKey(FString PartKey);

	UFUNCTION(BlueprintCallable)
	virtual bool IsAiming() { return false; } // TODO: Awkward. Needed for weapon control.
	UFUNCTION(BlueprintCallable)
	virtual bool IsFiring() { return false; } // TODO: Awkward. Needed for weapon control.

	FVector GetWorldLookLocation() { return this->WorldLookLocation; }

	UAnchorComponent* GetEye() { return this->Eye; }

	UFUNCTION(BlueprintCallable)
	virtual UInventoryComponent* GetInventory() { return this->Inventory; }

	UFUNCTION(BlueprintCallable)
	UProceduralAnimationComponent* GetProceduralAnimator() { return this->ProceduralAnimator; }

	URobotCameraController* GetCameraController() { return this->CameraController; }

	UFUNCTION(BlueprintCallable)
	TArray<URobotPartCollider*> GetParts() { return this->Parts; }

	UFUNCTION(BlueprintCallable)
	float GetMaxHealth();
	UFUNCTION(BlueprintCallable)
	float GetHealth();
	UFUNCTION(BlueprintCallable)
	int GetPersistentDamageCount();

	UFUNCTION(BlueprintCallable)
	float GetMaxEnergy() { return this->MaxEnergy; }
	UFUNCTION(BlueprintCallable)
	float GetEnergy() { return this->Energy; }

	UFUNCTION(BlueprintCallable)
	float GetMaxStamina() { return this->MaxStamina; }
	UFUNCTION(BlueprintCallable)
	float GetStamina() { return this->Stamina; }

private:
	void InputMoveForward(float Value);
	void InputMoveRight(float Value);
	
	void InputLookUp(float Value);
	void InputLookRight(float Value);

	void InputStartZoom();
	void InputEndZoom();

	void InputInteract();

	void InputToggleAttachments();

public: // For Projectile.
	void ServerRegisterDamage(URobotPartCollider* PrimaryPart, FDamageProfile DamageProfile);
private:
	void ServerOnHealthChanged();
	UFUNCTION(NetMulticast, Reliable)
	void ClientUpdateHealth(const FString& PartKey, float NewPartHealth, bool HasPersistentDamage);

	void ApplyToggleAttachments();
	UFUNCTION(Server, Reliable)
	void ServerToggleAttachments(bool State);
	UFUNCTION(NetMulticast, Reliable)
	void ClientToggleAttachments(bool State);

	UFUNCTION(Server, Unreliable)
	void ServerSetWorldLookLocation(FVector Update);

	UFUNCTION(Server, Reliable)
	void ServerPlayAnimation(FProceduralAnimationRequest Request);
	UFUNCTION(NetMulticast, Reliable)
	void ClientPlayAnimation(FProceduralAnimationRequest Request);

	void AnimationApplyMoveItem(FProceduralAnimation* Animation);
	void ApplyMoveItem(AItemActor* Item, UInventorySlotComponent* Slot);
	UFUNCTION(Server, Reliable)
	void ServerApplyMoveItem(AItemActor* Item, const FString& SlotKey);
	UFUNCTION(NetMulticast, Reliable)
	void ClientApplyMoveItem(AItemActor* Item, const FString& SlotKey);

	void AnimationApplyTakeItem(FProceduralAnimation* Animation);
	void ApplyTakeItem(AItemActor* Item, UInventorySlotComponent* Slot);
	UFUNCTION(Server, Reliable)
	void ServerApplyTakeItem(AItemActor* Item, const FString& SlotKey);
	UFUNCTION(NetMulticast, Reliable)
	void ClientApplyTakeItem(AItemActor* Item, const FString& SlotKey);
	
	void AnimationApplyDropItem(FProceduralAnimation* Animation);
	void ApplyDropItem(AItemActor* Item);
public: // TODO: For BatteryItem, understand implication.
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerDropItem(AItemActor* Item);
private:
	UFUNCTION(NetMulticast, Reliable)
	void ClientDropItem(AItemActor* Item);

	void AnimationApplyUseItem(FProceduralAnimation* Animation);
	UFUNCTION(Server, Reliable)
	void ServerUseItem();

protected:
	UFUNCTION(Server, Reliable)
	void ServerFire();

};
