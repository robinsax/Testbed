#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "../Robots/RobotPartCollider.h"

#include "ProjectileActor.generated.h"

UCLASS()
class AProjectileActor : public AActor {
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	FDamageProfile DamageProfile;

	UProjectileMovementComponent* Movement;

public:
	AProjectileActor();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

public:
	void ServerInitialize();

private:
	UFUNCTION()
	void ServerHandleHit(const FHitResult& Hit);
};
