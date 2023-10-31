#include "ProjectileActor.h"

#include "../Util.h"
#include "../Robots/BaseRobot.h"

AProjectileActor::AProjectileActor() {
	PrimaryActorTick.bCanEverTick = true;
}

void AProjectileActor::BeginPlay() {
	Super::BeginPlay();
	
	this->Movement = this->GetComponentByClass<UProjectileMovementComponent>();
}

void AProjectileActor::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AProjectileActor::ServerInitialize() {
	this->SetReplicates(true);

	this->Movement = this->GetComponentByClass<UProjectileMovementComponent>();
	this->Movement->OnProjectileStop.AddDynamic(this, &AProjectileActor::ServerHandleHit);
	this->Movement->Velocity = this->GetActorForwardVector() * Movement->InitialSpeed;
	this->Movement->Activate();
}

void AProjectileActor::ServerHandleHit(const FHitResult& Hit) {
	URobotPartCollider* HitPart = Cast<URobotPartCollider>(Hit.GetComponent());
	if (IsValid(HitPart)) {
		SANITY_LOG_D("HandleHit: %s", *HitPart->GetName());

		HitPart->GetRobot()->ServerRegisterDamage(HitPart, this->DamageProfile);
	}

	this->Destroy();
}
