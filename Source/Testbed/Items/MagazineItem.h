#pragma once

#include "CoreMinimal.h"

#include "../Projectiles/ProjectileActor.h"
#include "ItemActor.h"

#include "MagazineItem.generated.h"

UCLASS()
class AMagazineItem : public AItemActor {
	GENERATED_BODY()
	
private:
	UPROPERTY(EditDefaultsOnly, Category="Magazine")
	FString MagazineType; // UENUM: ?

	UPROPERTY(EditDefaultsOnly, Category="Magazine")
	TSubclassOf<AProjectileActor> ProjectileType; // TODO: tmp.

public:
	TSubclassOf<AProjectileActor> ServerGetNextProjectileType();

	FString GetMagazineType() { return this->MagazineType; }

};
