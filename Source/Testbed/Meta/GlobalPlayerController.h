#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Engine/PostProcessVolume.h"

#include "../Robots/BaseRobot.h"

#include "GlobalPlayerController.generated.h"

class AGlobalHUD;

UENUM(BlueprintType)
enum class EPlayerControlMode : uint8 {
	None,
	Normal,
	Inventory
};

USTRUCT()
struct FScreenRaycastResult {
	GENERATED_BODY()

	FVector EndLocation;
	AActor* Actor;
	USceneComponent* Component;
};

UCLASS()
class AGlobalPlayerController : public APlayerController {
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category="Control")
	FVector2D LookSensitivity;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<AActor> InventoryWorldCursorType;

	EPlayerControlMode Mode;

	AActor* InventoryWorldCursor;

	bool InInventory;
	
	bool ShowPersistentDamage;

	bool MovingItem;
	AItemActor* CurrentInventoryItem;
	UInventorySlotComponent* CurrentInventorySlot;

	APostProcessVolume* NormalOutlineVolume;
	APostProcessVolume* DamageOutlineVolume;

public:
	static AGlobalPlayerController* Get(UWorld* World);
	static EPlayerControlMode GetMode(UWorld* World);

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

protected:
	virtual void BeginPlay() override;

private:
	AGlobalHUD* GetGlobalHUD();

	void InputStartViewInventory();
	void InputEndViewInventory();

	void InputStartMouse1();
	void InputEndMouse1();

	void InputStartMouse2();
	void InputEndMouse2();

public:
	FVector2D NormalizedMousePosition();
	FScreenRaycastResult ScreenRaycast(FVector2D NormScreenPosition, float MaxDistance);
	FScreenRaycastResult ScreenRaycast(FVector2D NormScreenPosition, float MaxDistance, TArray<AActor*> IgnoreActors, ECollisionChannel Channel);

	bool IsSlotDropTarget(UInventorySlotComponent* Slot);
	bool IsItemManageTarget(AItemActor* Item);

	AItemActor* GetManageItemTarget();

	EPlayerControlMode GetMode();
	ABaseRobot* GetRobot();
	FVector2D GetLookSensitivity();

	bool ShouldShowPeristentDamage() { return this->ShowPersistentDamage; }

};
