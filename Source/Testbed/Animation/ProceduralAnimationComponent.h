#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "ProceduralAnimation.h"

#include "ProceduralAnimationComponent.generated.h"

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UProceduralAnimationComponent : public UActorComponent {
	GENERATED_BODY()

private:
	FProceduralAnimationRequest CurrentRequest; // TODO: No longer need request param within blueprints.
	FProceduralAnimation CurrentBuild;

protected:
	UFUNCTION(BlueprintNativeEvent)
	FProceduralAnimation GetProceduralAnimation(FProceduralAnimationRequest Request);

public:
	FProceduralAnimation TryGetProceduralAnimation(FProceduralAnimationRequest Request);

	UFUNCTION(BlueprintCallable)
	void ProcAnimStart();
	UFUNCTION(BlueprintCallable)
	FProceduralAnimation ProcAnimFinish();
	UFUNCTION(BlueprintCallable)
	FProceduralAnimation ProcAnimNone();

	UFUNCTION(BlueprintCallable)
	void ProcAnimMoveToAnchor(FString TargetName, FString AnchorName, float LerpRate);
	UFUNCTION(BlueprintCallable)
	void ProcAnimWait(float Time);
	UFUNCTION(BlueprintCallable)
	void ProcAnimInvokeAction();
	UFUNCTION(BlueprintCallable)
	void ProcAnimAttachItemTo(FString TargetName, FString Item);

	UFUNCTION(BlueprintCallable)
	bool ProcAnimIsType(FString Type);
	UFUNCTION(BlueprintCallable)
	AActor* ProcAnimGetContextActor(FProceduralAnimationRequest Request, FString Name);

};
