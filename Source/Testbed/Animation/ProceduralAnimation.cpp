#include "ProceduralAnimation.h"

#include "../Util.h"
#include "../Items/InventoryComponent.h"
#include "../Items/InventorySlotComponent.h"
#include "BaseRobotAnimInstance.h" // To finalize friend.

FProceduralAnimationKey::FProceduralAnimationKey(int SetType) {
    this->Type = SetType;
}

FProceduralAnimationRequest::FProceduralAnimationRequest(FString SetType, int SetLevel) {
    this->Type = SetType;
    this->Level = SetLevel;
}

void FProceduralAnimationRequest::SetContextActor(FString Name, AActor* Actor) {
    if (this->ActorOneName.Len() == 0) {
        this->ActorOneName = Name;
        this->ActorOne = Actor;
    }
    else if (this->ActorTwoName.Len() == 0) {
        this->ActorTwoName = Name;
        this->ActorTwo = Actor;
    }
    else {
        SANITY_ERR("SetContextActor: Context full");
    }
}

void FProceduralAnimationRequest::SetContextComponentWithKey(FString Name, FString Key) {
    if (this->ComponentOneName.Len() == 0) {
        this->ComponentOneName = Name;
        this->ComponentOne = Key;
    }
    else if (this->ComponentTwoName.Len() == 0) {
        this->ComponentTwoName = Name;
        this->ComponentTwo = Key;
    }
    else {
        SANITY_ERR("SetContextComponent: Context full");
    }
}

void FProceduralAnimationRequest::SetContextComponent(FString Name, UAnchorComponent* Component) {
    this->SetContextComponentWithKey(Name, Component->GetKey());
}

void FProceduralAnimationRequest::SetContextComponent(FString Name, UInventorySlotComponent* Component) {
    this->SetContextComponentWithKey(Name, Component->GetKey());
}

AActor* FProceduralAnimationRequest::GetContextActor(FString Name) {
    if (this->ActorOneName.Equals(Name)) return this->ActorOne;
    if (this->ActorTwoName.Equals(Name)) return this->ActorTwo;
    return nullptr;
}

USceneComponent* FProceduralAnimationRequest::GetContextComponent(AActor* Owner, FString Name) {
    FString Key;
    if (this->ComponentOneName == Name) {
        Key = this->ComponentOne;
    }
    else if (this->ComponentTwoName == Name) {
        Key = this->ComponentTwo;
    }
    else {
        return nullptr;
    }

    UAnchorComponent* Anchor = UAnchorComponent::FindOne(Owner, Key);
    if (IsValid(Anchor)) return Anchor;

    IInventoryOwner* WithInventory = Cast<IInventoryOwner>(Owner);
    if (WithInventory == nullptr) return nullptr;

    return WithInventory->GetInventory()->GetSlotByKey(Key);
}
