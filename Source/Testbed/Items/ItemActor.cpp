#include "ItemActor.h"

#include "Net/UnrealNetwork.h"

#include "../Util.h"
#include "../Robots/BaseRobot.h"
#include "../Meta/GlobalPlayerController.h"
#include "InventoryComponent.h"

AItemActor::AItemActor() {
	PrimaryActorTick.bCanEverTick = true;
}

void AItemActor::BeginPlay() {
	Super::BeginPlay();
	
	this->Mesh = this->GetComponentByClass<UStaticMeshComponent>();
	this->Collider = Cast<UPrimitiveComponent>(this->RootComponent);

	this->ProceduralAnimator = this->GetComponentByClass<UProceduralAnimationComponent>();
	this->Inventory = this->GetComponentByClass<UInventoryComponent>();
	
    this->RotationLerp.SetBaseValue(FRotator());
}

void AItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AItemActor, Takeable, COND_None);
}

void AItemActor::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if (this->GetLocalRole() != ROLE_Authority) {
		AGlobalPlayerController* Controller = AGlobalPlayerController::Get(this->GetWorld());
		if (!ASSERT_PTR_SANITY(Controller, "InventorySlotComponent tick: Controller")) return;

		this->Mesh->SetRenderCustomDepth(Controller->IsItemManageTarget(this));
	}

	if (this->InventoryCollisionEnabled) {
		// TODO: Location control awkward.
		ABaseRobot* EquippedTo = this->GetRobotEquippedTo();
		if (IsValid(EquippedTo)) {
			this->SetActorRotation(this->RotationLerp.Update(DeltaTime));

			UAnchorComponent* Grip = UAnchorComponent::FindOne(this, TEXT("Grip"));
			FVector LocalOffset = this->GetActorRotation().RotateVector(Grip->GetRelativeLocation());
			this->SetActorLocation(EquippedTo->GetInventory()->GetEquipSlot()->GetComponentLocation() - LocalOffset);
		}
	}
}

void AItemActor::ServerUseOnRobot_Implementation(ABaseRobot* Robot) {
	SANITY_ERR_D("ServerUseOnRobot: Unimplemented for %s", *this->GetName());
}

void AItemActor::ServerSetTakeable(bool Update) {
	this->Takeable = Update;
}

void AItemActor::SetTargetRotation(FRotator TargetRotation) {
	this->RotationLerp.SetTargetValue(TargetRotation);
}

void AItemActor::SetInventoryCollision(bool Enabled) {
	this->InventoryCollisionEnabled = Enabled;
	this->Collider->SetSimulatePhysics(!Enabled);
	this->Collider->SetCollisionProfileName(Enabled ? TEXT("ItemInInventory") : TEXT("Item"), true);
}

ABaseRobot* AItemActor::GetRobotEquippedTo() {
	if (!IsValid(this->Inventory)) {
		// TODO: Support needed?
		SANITY_ERR("Checking IsEquipped on simple item");
		return nullptr;
	}

	UInventoryComponent* TopInventory = this->Inventory;
	while (IsValid(TopInventory->GetParentInventory())) {
		TopInventory = TopInventory->GetParentInventory();
	}

	ABaseRobot* Robot = Cast<ABaseRobot>(TopInventory->GetOwner());
	if (!IsValid(Robot)) return nullptr;

	if (Robot->GetEquippedItem() != this) return nullptr;
	return Robot;
}

FString AItemActor::GetRequiredSlotType() {
	if (!this->Takeable) return TEXT("NEVER");

	return this->RequiredSlotType;
}
