#include "RobotCameraController.h"

#include "Kismet/KismetMathLibrary.h"

#include "../Util.h"
#include "GlobalPlayerController.h"

URobotCameraController::URobotCameraController() {
	PrimaryComponentTick.bCanEverTick = true;
}

void URobotCameraController::BeginPlay() {
	Super::BeginPlay();

	this->SpringArm = this->GetOwner()->GetComponentByClass<USpringArmComponent>();
	this->Camera = this->GetOwner()->GetComponentByClass<UCameraComponent>();

	this->CameraRotationLerp.SetBaseValue(this->Camera->GetRelativeRotation());
	this->CameraFOVLerp.SetBaseValue(this->Camera->FieldOfView);
	this->SpringArmLengthLerp.SetBaseValue(this->SpringArm->TargetArmLength);
	this->SpringArmOffsetLerp.SetBaseValue(this->SpringArm->SocketOffset);
	this->SpringArmRotationLerp.SetBaseValue(this->GetOwner()->GetActorRotation());
	this->CameraRollLerp.SetBaseValue(0.0f);
}

void URobotCameraController::TickComponent(float DeltaTime, ELevelTick Type, FActorComponentTickFunction* Self) {
	if (this->GetOwner()->GetLocalRole() != ROLE_AutonomousProxy) return;

	Super::TickComponent(DeltaTime, Type, Self);

	float PrevInventoryView = this->InventoryView;
	this->InventoryView = AGlobalPlayerController::GetMode(this->GetWorld()) == EPlayerControlMode::Inventory;

	this->Camera->FieldOfView = this->CameraFOVLerp.Update(DeltaTime);
	this->Camera->SetRelativeRotation(this->CameraRotationLerp.Update(DeltaTime));
	this->SpringArm->TargetArmLength = this->SpringArmLengthLerp.Update(DeltaTime);
	this->SpringArm->SocketOffset = this->SpringArmOffsetLerp.Update(DeltaTime);
	this->SpringArm->SetWorldRotation(this->SpringArmRotationLerp.Update(DeltaTime));

	if (this->InventoryView) {
		if (!PrevInventoryView) {
			this->InventoryViewRotation = this->GetOwner()->GetActorRotation();
			this->InventoryViewRotation.Yaw += 180.0f;
		}
	
		this->SpringArmRotationLerp.SetTargetValue(this->InventoryViewRotation);
		this->SpringArmOffsetLerp.SetTargetValue(this->CameraInventorySocketOffset);
		this->SpringArmLengthLerp.SetTargetValue(this->CameraInventoryDistance);
		this->CameraRotationLerp.ResetTargetValue();
		this->CameraFOVLerp.ResetTargetValue();
	}
	else {
		if (PrevInventoryView) {
			this->CameraRotationLerp.ResetValueHard();
		}
		FVector RelativeSpringArmOffset;
		
		if (this->Zoom) {
			this->CameraFOVLerp.SetTargetValue(this->CameraZoomFOV);
		}
		else if (this->Sprint) {
			this->CameraFOVLerp.SetTargetValue(this->CameraSprintFOV);
			RelativeSpringArmOffset.X += this->CameraSprintPull;
		}
		else {
			this->CameraFOVLerp.ResetTargetValue();
		}

		RelativeSpringArmOffset.Y = FMath::Abs(this->Lean) * this->CameraLeanPushDistance;
		RelativeSpringArmOffset.Y -= FMath::Clamp(this->Lean, -1.0f, 0.0f) * this->SpringArmOffsetLerp.GetBaseValue().Y;
		if (this->Lean < 0.0f) RelativeSpringArmOffset.Y *= -1;

		FRotator CurrentRotation = this->CameraRotationLerp.GetValue();

		float Pitch = FMath::Clamp(CurrentRotation.Pitch + this->PitchInput, -this->CameraPitchClamp, this->CameraPitchClamp);
		this->PitchInput = 0.0f;

		float RelativePitch = Pitch / this->CameraPitchClamp;

		this->CameraRollLerp.SetTargetValue(this->Lean * this->CameraLeanRotation);
		CurrentRotation.Roll = this->CameraRollLerp.Update(DeltaTime);
		CurrentRotation.Pitch = Pitch;
		this->CameraRotationLerp.SetValueHard(CurrentRotation);
		
		RelativeSpringArmOffset.Z = -RelativePitch * this->CameraPitchPullHeight;

		this->SpringArmLengthLerp.SetRelativeTargetValue(FMath::Abs(RelativePitch) * -this->CameraPitchPullDistance);
		this->SpringArmOffsetLerp.SetRelativeTargetValue(RelativeSpringArmOffset);
		this->SpringArmRotationLerp.SetValueHard(this->GetOwner()->GetActorRotation());
	}
}

void URobotCameraController::UpdateCameraPitch(float Amount) {
	this->PitchInput += Amount;
}

void URobotCameraController::SetZoom(bool NewZoom) {
	this->Zoom = NewZoom;
}

void URobotCameraController::SetLean(float NewLean) {
	this->Lean = NewLean;
}

void URobotCameraController::SetSprint(bool NewSprint) {
	this->Sprint = NewSprint;
}

FVector URobotCameraController::GetCameraLocation() {
	return this->Camera->GetComponentLocation();
}

FRotator URobotCameraController::GetCameraRelativeRotation() {
	return this->Camera->GetRelativeRotation();
}
