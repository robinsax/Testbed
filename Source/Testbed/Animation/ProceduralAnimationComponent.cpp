#include "ProceduralAnimationComponent.h"

#include "../Util.h"

FProceduralAnimation UProceduralAnimationComponent::GetProceduralAnimation_Implementation(FProceduralAnimationRequest Request) {
	return this->ProcAnimNone();
}

FProceduralAnimation UProceduralAnimationComponent::TryGetProceduralAnimation(FProceduralAnimationRequest Request) {
	this->CurrentRequest = Request;

	return this->GetProceduralAnimation(Request);
}

void UProceduralAnimationComponent::ProcAnimStart() {
	this->CurrentBuild = FProceduralAnimation();
}

FProceduralAnimation UProceduralAnimationComponent::ProcAnimFinish() {
	return this->CurrentBuild;
}

FProceduralAnimation UProceduralAnimationComponent::ProcAnimNone() {
	SANITY_WARN_D("None animation: %s on %s", *this->CurrentRequest.Type, *this->GetOwner()->GetName());

	this->ProcAnimStart();
	this->ProcAnimWait(5.0f);
	return this->ProcAnimFinish();
}

void UProceduralAnimationComponent::ProcAnimMoveToAnchor(FString TargetName, FString AnchorName, float LerpRate) {
	FProceduralAnimationKey Key(PAK_MoveToAnchor);
	Key.AnchorName = AnchorName;
	Key.TargetName = TargetName;
	Key.LerpRate = LerpRate;
	this->CurrentBuild.Keys.Push(Key);
}

void UProceduralAnimationComponent::ProcAnimWait(float Time) {
	FProceduralAnimationKey Key(PAK_Wait);
	Key.WaitTime = Time;
	this->CurrentBuild.Keys.Push(Key);
}

void UProceduralAnimationComponent::ProcAnimInvokeAction() {
	FProceduralAnimationKey Key(PAK_InvokeAction);
	this->CurrentBuild.Keys.Push(Key);
}

void UProceduralAnimationComponent::ProcAnimAttachItemTo(FString TargetName, FString Item) {
	FProceduralAnimationKey Key(PAK_AttachItemTo);
	Key.AnchorName = TargetName;
	Key.TargetName = Item;
	this->CurrentBuild.Keys.Push(Key);
}

bool UProceduralAnimationComponent::ProcAnimIsType(FString Type) {
	return this->CurrentRequest.Type.Equals(Type);
}

AActor* UProceduralAnimationComponent::ProcAnimGetContextActor(FProceduralAnimationRequest Request, FString Name) {
	return Request.GetContextActor(Name);
}
