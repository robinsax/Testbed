#include "BaseRobotAnimInstance.h"

#include "../Util.h"
#include "../KeyableComponent.h"
#include "../Robots/BaseRobot.h"
#include "../Items/ItemActor.h"
#include "../Items/InventoryComponent.h"
#include "../Items/InventorySlotComponent.h"

// todo no
#include "../Robots/HumanoidRobot.h"

void UBaseRobotAnimInstance::BindToRobot(ABaseRobot* TargetRobot) {
    this->BaseRobot = TargetRobot;

    for (int Idx = 0; Idx < PARL_MAX; Idx++) {
        this->CurrentAnimations.Push(FProceduralAnimation());
    }
}

void UBaseRobotAnimInstance::ApplyAnimationMove(FString TargetName, FVector WorldLocation, USceneComponent* Anchor) {
}

USceneComponent* UBaseRobotAnimInstance::ResolveAnimationContextComponent(FString Name, FProceduralAnimation* Animation) {
    AActor* Parent = this->BaseRobot;

    if (Name.Contains(TEXT(":"))) {
        TArray<FString> Parts;
        Name.ParseIntoArray(Parts, TEXT(":"));

        #ifdef SANITY_CHECKS
        if (Parts.Num() != 2) {
            SANITY_WARN_D("Invalid format %s, in %s key %d", *Name, *Animation->Request.Type, Animation->CurrentKey);
            return nullptr;
        }
        if (!Parts[0].StartsWith(TEXT("<")) || !Parts[0].EndsWith(TEXT(">"))) {
            SANITY_WARN_D("Invalid actor %s, in %s key %d", *Parts[0], *Animation->Request.Type, Animation->CurrentKey);
            return nullptr;
        }
        #endif

        Name = Parts[1];

        FString ContextParentName = Parts[0].Mid(1, Parts[0].Len() - 2);
        Parent = Animation->Request.GetContextActor(ContextParentName);
        if (!IsValid(Parent)) {
            SANITY_WARN_D("Invalid parent %s, in %s key %d", *ContextParentName, *Animation->Request.Type, Animation->CurrentKey);
            return nullptr;
        }
    }

    
    if (Name.StartsWith(TEXT("<"))) {
        #ifdef SANITY_CHECKS
        if (!Name.EndsWith(TEXT(">"))) {
            SANITY_WARN_D("Invalid component format %s, in %s key %d", *Name, *Animation->Request.Type, Animation->CurrentKey);
            return nullptr;
        }
        #endif

        FString ContextComponentName = Name.Mid(1, Name.Len() - 2);
        USceneComponent* Component = Animation->Request.GetContextComponent(Parent, ContextComponentName);
        if (!IsValid(Component)) {
            SANITY_WARN_D("Invalid component %s (not in context), in %s key %d", *ContextComponentName, *Animation->Request.Type, Animation->CurrentKey);
            return nullptr;
        }

        UInventorySlotComponent* Slot = Cast<UInventorySlotComponent>(Component);
        if (IsValid(Slot)) {
            FString ProxyAnchor = Slot->GetAnimationProxyAnchor();
            if (ProxyAnchor.Len() > 0) {
                return UAnchorComponent::FindOne(Parent, ProxyAnchor);
            }
        }
        return Component;
    }

    return UAnchorComponent::FindOne(Parent, Name);
}

bool UBaseRobotAnimInstance::ApplyAnimationKey(FProceduralAnimation* Animation, int KeyIndex, float DeltaTime) {
    FProceduralAnimationKey* Key = &Animation->Keys[KeyIndex];
    bool Current = Animation->CurrentKey == KeyIndex;

    int K = 0;
    FVector Location;
    USceneComponent* Component = nullptr;
    AItemActor* Item = nullptr;

    switch (Key->Type) {
        case PAK_InvokeAction:
            if (Current) Animation->Action.ExecuteIfBound(Animation);
            return true;
        case PAK_MoveToAnchor:
            Component = this->ResolveAnimationContextComponent(Key->AnchorName, Animation);
            if (!IsValid(Component)) {
                SANITY_ERR_D("MoveToAnchor anchor %s invalid, in animation key %d", *Key->AnchorName, KeyIndex);
                return true;
            }

            Location = this->GetSkelMeshComponent()->GetComponentTransform().InverseTransformPosition(Component->GetComponentLocation());
            this->ApplyAnimationMove(Key->TargetName, Location, Component);

            return true;
        case PAK_AttachItemTo:
            Item = Cast<AItemActor>(Animation->Request.GetContextActor(Key->TargetName));
            if (!IsValid(Item)) {
                SANITY_ERR("Invalid attach item in animation");
                return true;
            }

            Component = this->ResolveAnimationContextComponent(Key->AnchorName, Animation);
            if (!IsValid(Component)) {
                SANITY_ERR("Invalid component reference in animation AttachItem");
                return true;
            }

            // TODO: Drop if canceled.

            Item->SetInventoryCollision(true);
            Item->AttachToComponent(Component, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
            return true;
        case PAK_Wait:
            K = KeyIndex - 1;
            while (K >= 0 && Animation->Keys[K].Type != PAK_Wait) {
                this->ApplyAnimationKey(Animation, K, DeltaTime);
                K--;
            }
            Key->WaitTime -= DeltaTime;
            return Key->WaitTime <= 0.0f;
        case PAK_None:
        default:
            SANITY_ERR("Unknown animation key type");
            return true;
    }
}

void UBaseRobotAnimInstance::Tick(float DeltaTime) {
    for (int Idx = 0; Idx < this->CurrentAnimations.Num(); Idx++) {
        FProceduralAnimation* Animation = &this->CurrentAnimations[Idx];
        if (Animation->Keys.Num() == 0) continue;

        if (Animation->CurrentKey >= Animation->Keys.Num()) {
            this->CurrentAnimations[Idx] = FProceduralAnimation();
            continue;
        }

        bool KeyDone = this->ApplyAnimationKey(Animation, Animation->CurrentKey, DeltaTime);
        if (KeyDone) Animation->CurrentKey++;
    }
}

void UBaseRobotAnimInstance::PushAnimation(FProceduralAnimation Animation) {
    SANITY_LOG_D(
        "PushAnimation: Level=%d Type=%s; Actors=%s,%s; Components=%s,%s",
        Animation.Request.Level,
        *Animation.Request.Type,
        *Animation.Request.ActorOneName,
        *Animation.Request.ActorTwoName,
        *Animation.Request.ComponentOneName,
        *Animation.Request.ComponentTwoName
    );

    if (this->CurrentAnimations[Animation.Request.Level].Keys.Num() > 0) {
        // TODO: Cancel somehow.
    }
    this->CurrentAnimations[Animation.Request.Level] = Animation;
}

bool UBaseRobotAnimInstance::HasActiveAnimationAtLevel(int Level) {
    return this->CurrentAnimations[Level].Keys.Num() > 0;
}
