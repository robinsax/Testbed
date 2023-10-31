#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"

#include "../Lerp.h"
#include "ProceduralAnimation.h"
#include "BaseRobotAnimInstance.h"

#include "HumanoidRobotAnimInstance.generated.h"

class AHumanoidRobot;

UCLASS()
class UHumanoidRobotAnimInstance : public UBaseRobotAnimInstance {
	GENERATED_BODY()

private:
	int Step;
	float StepTime;
	float CurrentStepDuration;
	FVector StepTarget;

	TArray<int> MustStep;

	TArray<FString> CurrentAnimationEffectorOverrides;

	TArray<FVector> TargetLegEffectors;
	TArray<FVector> TargetArmEffectors;
	TArray<int> ArmEffectorTicksSinceAnimate;

protected:
	UPROPERTY(BlueprintReadOnly)
	AHumanoidRobot* Robot;

	UPROPERTY(BlueprintReadOnly)
	float StepAlpha;

	UPROPERTY(BlueprintReadOnly)
	float Lean;
	UPROPERTY(BlueprintReadOnly)
	bool Crouching;
	UPROPERTY(BlueprintReadOnly)
	bool Falling;
	UPROPERTY(BlueprintReadOnly)
	float TorsoYaw;

	UPROPERTY(BlueprintReadOnly)
	FVector Velocity;
	UPROPERTY(BlueprintReadOnly)
	FRotator Rotation;

	UPROPERTY(BlueprintReadOnly)
	TArray<FVector> LegEffectors;
	UPROPERTY(BlueprintReadOnly)
	TArray<FVector> ArmEffectors;

	FLerpedRotator WorldLookRotationLerp;
	UPROPERTY(BlueprintReadOnly)
	FRotator WorldLookRotation;

	FLerpedFloat TorsoYawLerp;

public:
	virtual void BindToRobot(ABaseRobot* TargetRobot) override;
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void ApplyAnimationMove(FString Target, FVector Location, USceneComponent* Anchor) override;

private:
	void TickLegIK(float DeltaTime);

};
