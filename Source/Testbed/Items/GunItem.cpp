#include "GunItem.h"

#include "Kismet/KismetMathLibrary.h"

#include "../Util.h"
#include "../Robots/BaseRobot.h"
#include "../Projectiles/ProjectileActor.h"

AGunItem::AGunItem() {
    PrimaryActorTick.bCanEverTick = true;
}

void AGunItem::BeginPlay() {
    Super::BeginPlay();

    this->Inventory->RefreshSlots(); // Ensure slots loaded.

    // TODO: Will break if gun spawns in inventory.
    this->MagazineWell = this->Inventory->GetSlotByKey(TEXT("Slot:Magazine"));
    CHECK_BIND_PTR_SANITY(this->MagazineWell, "AGunItem: MagazineWell");

    this->Muzzle = UAnchorComponent::FindOne(this, TEXT("Muzzle"));
    CHECK_BIND_PTR_SANITY(this->Muzzle, "AGunItem: Muzzle");
}

void AGunItem::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    if (this->GetLocalRole() == ROLE_Authority) {
        if (this->CooldownTimer > 0.0f) {
            this->CooldownTimer -= DeltaTime;
        }
    }

    this->RotationOverride = false;

    ABaseRobot* EquippedTo = this->GetRobotEquippedTo();
    if (!IsValid(EquippedTo) || (!EquippedTo->IsAiming() && !EquippedTo->IsFiring())) return;

    this->SetTargetRotation(UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), EquippedTo->GetWorldLookLocation()));
    this->RotationOverride = true;
}

void AGunItem::LoadMagazine(AMagazineItem* Magazine) {
    Magazine->SetInventoryCollision(true);
    this->MagazineWell->SetItem(Magazine);
}

void AGunItem::ServerFire() {
    if (this->CooldownTimer > 0.0f) {
        return;
    }

    AMagazineItem* Magazine = Cast<AMagazineItem>(this->MagazineWell->GetItem());
    if (!ASSERT_PTR_SANITY(Magazine, "ServerFire: Magazine")) return;

    this->CooldownTimer = 0.2f; // TODO: no.

    TSubclassOf<AProjectileActor> ProjectileType = Magazine->ServerGetNextProjectileType();

    AProjectileActor* Projectile = this->GetWorld()->SpawnActor<AProjectileActor>(ProjectileType, this->Muzzle->GetComponentLocation(), this->GetActorRotation());
    Projectile->ServerInitialize();
}

void AGunItem::SetTargetRotation(FRotator TargetRotation) {
    if (this->RotationOverride) return;

    Super::SetTargetRotation(TargetRotation);
}

bool AGunItem::CanFire() {
    // TODO: Update when magazines have cap.
    return IsValid(Cast<AMagazineItem>(this->MagazineWell->GetItem())); 
}
