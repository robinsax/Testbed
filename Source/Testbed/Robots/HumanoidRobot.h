#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"

#include "../Lerp.h"
#include "../Animation/HumanoidRobotAnimInstance.h"
#include "../Items/InventorySlotComponent.h"
#include "../Items/MagazineItem.h"
#include "../HUD/RobotHUDWidget.h"
#include "BaseRobot.h"

#include "HumanoidRobot.generated.h"

// TODO: UENUM
#define HRMS_Walking 1
#define HRMS_Sprinting 2
#define HRMS_Aiming 4
#define HRMS_Crouching 8
#define HRMS_InInventory 16
#define HRMS_Hipfire 32

UCLASS()
class AHumanoidRobot : public ABaseRobot {
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category="Rig Config")
	TArray<FVector> LegOffsets;
	UPROPERTY(EditDefaultsOnly, Category="Rig Config")
	float StepDuration = 0.2f;
	UPROPERTY(EditDefaultsOnly, Category="Rig Config")
	float StepOffsetTolerance = 5.0f;
	UPROPERTY(EditDefaultsOnly, Category="Rig Config")
	float StepVelocityScale = 0.2f;
	UPROPERTY(EditDefaultsOnly, Category="Rig Config")
	float StepVerticalHeight = 4.0f;
	UPROPERTY(EditDefaultsOnly, Category="Rig Config")
	float CrouchStepScale = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category="Rig Config")
	float JumpLegOffset = 60.0f;

	UPROPERTY(EditDefaultsOnly, Category="Rig Config")
	TArray<FVector> ArmOffsets;

private:
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float JumpStrength = 400.0f;
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float StrafeAngle = 65.0f;
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float StrafeSpeedModifier = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float WalkSpeedModifier = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float CrouchSpeedModifier = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float CrouchHeightModifier = 0.8f;
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float CrouchMeshVerticalOffset = 10.0f;
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float MaxLeanDegrees = 20.0f;
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float SprintStaminaCost = 50.0f;
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float JumpStaminaCost = 50.0f;
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float NoEnergySpeedModifier = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category="Lerp")
	FLerpedFloat CrouchLerp;

	float BaseCapsuleHalfHeight;
	float BaseMeshVerticalOffset;

	float LeanInput;
	UPROPERTY(Replicated)
	float RepLean;
	float Lean;

	UPROPERTY(Replicated)
	bool RepFiring;
	bool Firing;

	UPROPERTY(Replicated)
	int RepMovementState;
	int MovementState; // UENUM: HRMS_*.
	int PrevMovementState;

	UPROPERTY(Replicated)
	bool RepCheckingStatus;
	bool CheckingStatus;

	UHumanoidRobotAnimInstance* AnimInstance;
	UWidgetComponent* WorldHUDWidget;

	TArray<AItemActor*> QuickSlots;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* Input) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	virtual void ControlTick(AGlobalPlayerController* PlayerController, float DeltaTime) override;
	virtual void SimulatedTick(float DeltaTime) override;

	virtual FVector ControlProcessMove(FVector Move) override;
	virtual void PlayBaseAnimation() override;

public:
	UFUNCTION(BlueprintCallable)
	bool IsInInventory();
	bool IsCrouching();
	virtual bool IsAiming() override;
	virtual bool IsFiring() override;
	bool IsSprinting();

	bool IsCheckingStatus() { return this->CheckingStatus; }

	float GetLean() { return this->Lean; }

public: // For player controller.
	void InputStartAim();
	void InputEndAim();

	void InputStartFire();
	void InputEndFire();

private:
	void InputStartCheckStatus();
	void InputEndCheckStatus();

	void InputQuickSlot(int Idx);
	void InputQuickSlot1() { this->InputQuickSlot(0); }
	void InputQuickSlot2() { this->InputQuickSlot(1); }
	void InputQuickSlot3() { this->InputQuickSlot(2); }
	void InputQuickSlot4() { this->InputQuickSlot(3); }
	void InputQuickSlot5() { this->InputQuickSlot(4); }

	void InputLeanRight(float Value);

	void InputStartCrouch();
	void InputEndCrouch();

	void InputStartSprint();
	void InputEndSprint();

	void InputStartJump();

	void InputReload();

	UFUNCTION(Server, Reliable)
	void ServerSetCheckingStatus(bool Update);

	void AnimationApplyReload(FProceduralAnimation* Animation);
	void ApplyReload(AMagazineItem* Magazine);
	UFUNCTION(Server, Reliable)
	void ServerApplyReload(AMagazineItem* Magazine);
	UFUNCTION(NetMulticast, Reliable)
	void ClientApplyReload(AMagazineItem* Magazine);

	UFUNCTION(Server, Reliable)
	void ServerSetMovementState(int Update);
	UFUNCTION(Server, Unreliable)
	void ServerSetLean(float Update);

	bool ApplyJump();
	UFUNCTION(Server, Reliable)
	void ServerStartJump();

	UFUNCTION(Server, Reliable)
	void ServerSetFiring(bool Update);

};
