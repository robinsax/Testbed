#include "MagazineItem.h"

TSubclassOf<AProjectileActor> AMagazineItem::ServerGetNextProjectileType() {
    return this->ProjectileType;
}
