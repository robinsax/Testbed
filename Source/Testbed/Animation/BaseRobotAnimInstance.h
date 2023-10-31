#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"

#include "ProceduralAnimation.h"

#include "BaseRobotAnimInstance.generated.h"

class ABaseRobot;

UCLASS()
class UBaseRobotAnimInstance : public UAnimInstance {
	GENERATED_BODY()

private:
	ABaseRobot* BaseRobot;

protected:
	TArray<FProceduralAnimation> CurrentAnimations;

public:
	virtual void Tick(float DeltaTime);
	virtual void BindToRobot(ABaseRobot* TargetRobot);
	virtual void PushAnimation(FProceduralAnimation Animation);

protected:
	virtual void ApplyAnimationMove(FString Target, FVector Location, USceneComponent* Anchor);

private:
	USceneComponent* ResolveAnimationContextComponent(FString Name, FProceduralAnimation* Animation);

	bool ApplyAnimationKey(FProceduralAnimation* Animation, int KeyIndex, float DeltaTime);

public:
	bool HasActiveAnimationAtLevel(int Level);

};
