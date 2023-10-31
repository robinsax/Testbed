#pragma once

#include "CoreMinimal.h"

#include "../KeyableComponent.h"
#include "ItemActor.h"
#include "MagazineItem.h"

#include "GunItem.generated.h"

UCLASS()
class AGunItem : public AItemActor {
	GENERATED_BODY()
	
private:
	UPROPERTY(EditDefaultsOnly, Category="Gun")
	FString RequiredMagazineType; // UENUM: AMagazineItem::MagazineType.

	// Components.
	UInventorySlotComponent* MagazineWell;
	UAnchorComponent* Muzzle;

	// State.
	bool RotationOverride;
	float CooldownTimer;

public:
	AGunItem();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

public:
	void LoadMagazine(AMagazineItem* Magazine);

	virtual void SetTargetRotation(FRotator TargetRotation) override;

	bool CanFire();
	FString GetRequiredMagazineType() { return this->RequiredMagazineType; }

	void ServerFire();

};
