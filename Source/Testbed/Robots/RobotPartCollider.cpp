#include "RobotPartCollider.h"

#include "Kismet/KismetMathLibrary.h"

#include "../Util.h"
#include "../Meta/GlobalPlayerController.h"
#include "../Robots/BaseRobot.h"

URobotPartCollider::URobotPartCollider() {
    PrimaryComponentTick.bCanEverTick = true;
}

void URobotPartCollider::BeginPlay() {
    Super::BeginPlay();

    // TODO: No.
    this->SetCollisionProfileName(TEXT("RobotPart"));
    this->ViewComponent = Cast<UPrimitiveComponent>(this->GetAttachChildren()[0]);
}

void URobotPartCollider::BindToRobot(ABaseRobot* TargetRobot) {
    this->Robot = TargetRobot;

    this->Health = this->MaxHealth;
    if (this->ProxyFor.Len() > 0) {
        this->ProxiedPart = TargetRobot->GetPartByKey(this->ProxyFor);
    }
}

void URobotPartCollider::TickComponent(float DeltaTime, ELevelTick Type, FActorComponentTickFunction* Self) {
    Super::TickComponent(DeltaTime, Type, Self);

    // TODO: Tmp.
    if (this->GetOwner()->GetLocalRole() != ROLE_AutonomousProxy) return;

    AGlobalPlayerController* PlayerController = AGlobalPlayerController::Get(this->GetWorld());
    if (!IsValid(PlayerController)) return;

    bool Damage = this->PersistentDamage;
    if (IsValid(this->ProxiedPart)) Damage = this->ProxiedPart->PersistentDamage;
    this->ViewComponent->SetRenderCustomDepth(PlayerController->ShouldShowPeristentDamage() && Damage);
}

float URobotPartCollider::ServerRegisterDamageAsPrimary(FDamageProfile Damage) {
    float HealthDamage = Damage.HealthDamage * this->HealthDamageModifier;
    bool CreatePersistentDamage = FMath::Rand() > (Damage.PersistentDamageChance * this->PersistentDamageChanceModifer);
    if (CreatePersistentDamage) {
        this->PersistentDamage = true;
    }

    if (this->Health > HealthDamage) {
        this->Health -= HealthDamage;
        return 0.0f;
    }
    else {
        float Overflow = HealthDamage - this->Health;
        this->Health = 0.0f;
        return Overflow;
    }
}

float URobotPartCollider::GetCurrentEnergyDrain() {
    if (!this->PersistentDamage) return 0.0f;

    return this->PersistentDamageEnergyDrain;
}

void URobotPartCollider::ServerRegisterDamageAsOverflow(float Damage) {
    this->Health = FMath::Max(this->Health - Damage, 0.0f);
}

void URobotPartCollider::ServerRegisterHeal(float Amount) {
    this->Health = FMath::Min(this->Health + Amount, this->MaxHealth);
}

void URobotPartCollider::ServerRegisterHealPersistentDamage() {
    this->PersistentDamage = false;
}

void URobotPartCollider::ClientUpdateHealth(float NewHealth, bool HasPersistentDamage) {
    this->Health = NewHealth;
    this->PersistentDamage = HasPersistentDamage;

    // Note client-side only.
    if (this->PersistentDamage && !IsValid(this->PersistentDamageEffect)) {
        this->PersistentDamageEffect = this->GetWorld()->SpawnActor<AActor>(
            this->PersistentDamageEffectType,
            FVector(),
            FRotator(),
            FActorSpawnParameters()
        );
        this->PersistentDamageEffect->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    }
    if (!this->PersistentDamage && IsValid(this->PersistentDamageEffect)) {
        this->PersistentDamageEffect->Destroy();
        this->PersistentDamageEffect = nullptr;
    }
}
