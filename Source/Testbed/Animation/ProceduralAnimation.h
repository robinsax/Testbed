#pragma once

#include "../KeyableComponent.h"

#include "ProceduralAnimation.generated.h"

class ABaseRobot;
class UInventorySlotComponent;
class UBaseRobotAnimInstance;

// TODO: Convert to UENUM.
#define PAK_None 0
#define PAK_MoveToAnchor 1
#define PAK_Wait 2
#define PAK_InvokeAction 3
#define PAK_AttachItemTo 4 // TODO: Rename.

// TODO: Convert to UENUM or bool.
#define PARL_Base 0
#define PARL_Interact 1
#define PARL_MAX 2

struct FProceduralAnimation;

DECLARE_DELEGATE_OneParam(CProceduralAnimationAction, FProceduralAnimation*);

USTRUCT(BlueprintType)
struct FProceduralAnimationKey {
    GENERATED_BODY()

public:
    int Type = 0;
    FString TargetName;

    FString AnchorName;
    float WaitTime = 0.0f;
    float LerpRate = 0.0f;

public:
    FProceduralAnimationKey() { }
    FProceduralAnimationKey(int SetType);

};

USTRUCT(BlueprintType)
struct FProceduralAnimationRequest {
    GENERATED_BODY()

    friend class UBaseRobotAnimInstance; // For debug logging.

public:
    UPROPERTY(BlueprintReadOnly)
    FString Type; // UENUM: Idle, TakeItem, MoveItem, DropItem, Reload
    UPROPERTY(BlueprintReadOnly)
    ABaseRobot* Robot = nullptr;

    UPROPERTY()
    int Level; // UENUM: PARL_*.

private:
    // TODO: Better way if we stick with this approach.
    // Animation context.
    UPROPERTY()
    FString ActorOneName = "";
    UPROPERTY()
    AActor* ActorOne = nullptr;

    UPROPERTY()
    FString ActorTwoName = "";
    UPROPERTY()
    AActor* ActorTwo = nullptr;

    UPROPERTY()
    FString ComponentOneName = "";
    UPROPERTY()
    FString ComponentOne = "";

    UPROPERTY()
    FString ComponentTwoName = "";
    UPROPERTY()
    FString ComponentTwo = "";

public:
    FProceduralAnimationRequest() { }
    FProceduralAnimationRequest(FString SetType, int SetLevel);

private:
    void SetContextComponentWithKey(FString Name, FString Key);

public:
    void SetContextActor(FString Name, AActor* Actor);
    void SetContextComponent(FString Name, UAnchorComponent* Component);
    void SetContextComponent(FString Name, UInventorySlotComponent* Component);

    AActor* GetContextActor(FString Name);
    USceneComponent* GetContextComponent(AActor* Owner, FString Name);

};

USTRUCT(BlueprintType)
struct FProceduralAnimation {
    GENERATED_BODY()

public:
    TArray<FProceduralAnimationKey> Keys;
    CProceduralAnimationAction Action;
    FProceduralAnimationRequest Request;

    int CurrentKey = 0;

};
