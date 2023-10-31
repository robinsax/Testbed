#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "../KeyableComponent.h"
#include "../Lerp.h"

#include "RobotCameraController.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TESTBED_API URobotCameraController : public UActorComponent {
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float CameraPitchClamp = 50.0f;
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float CameraPitchPullDistance = 100.0f;
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float CameraPitchPullHeight = 80.0f;
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float CameraLeanRotation = 10.0f;
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float CameraLeanPushDistance = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category="Control")
	FVector CameraInventorySocketOffset;
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float CameraInventoryDistance = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category="Control")
	float CameraZoomFOV = 40.0f;
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float CameraSprintFOV = 110.0f;
	UPROPERTY(EditDefaultsOnly, Category="Control")
	float CameraSprintPull = 80.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Lerp")
	FLerpedRotator CameraRotationLerp;
	UPROPERTY(EditDefaultsOnly, Category="Lerp")
	FLerpedFloat CameraFOVLerp;
	UPROPERTY(EditDefaultsOnly, Category="Lerp")
	FLerpedFloat SpringArmLengthLerp;
	UPROPERTY(EditDefaultsOnly, Category="Lerp")
	FLerpedVector SpringArmOffsetLerp;
	UPROPERTY(EditDefaultsOnly, Category="Lerp")
	FLerpedRotator SpringArmRotationLerp;
	UPROPERTY(EditDefaultsOnly, Category="Lerp")
	FLerpedFloat CameraRollLerp;

	FRotator InventoryViewRotation;

	float PitchInput;

	float Lean;

	bool Zoom;
	bool Sprint;

	bool InventoryView;

	USpringArmComponent* SpringArm;
	UCameraComponent* Camera;

public:	
	URobotCameraController();
	virtual void TickComponent(float DeltaTime, ELevelTick Type, FActorComponentTickFunction* Self) override;

protected:
	virtual void BeginPlay() override;

public:
	void UpdateCameraPitch(float Amount);
	void SetZoom(bool NewZoom);
	void SetSprint(bool NewSprint);
	void SetLean(float NewLean);

	FVector GetCameraLocation();
	FRotator GetCameraRelativeRotation();

};
