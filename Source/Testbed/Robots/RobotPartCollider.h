#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Blueprint/UserWidget.h"

#include "RobotPartCollider.generated.h"

class ABaseRobot;

USTRUCT()
struct FDamageProfile {
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float HealthDamage = 30.0f;
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float PersistentDamageChance = 0.25f;

};

UCLASS(Blueprintable)
class TESTBED_API URobotPartCollider : public UBoxComponent {
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category="Robot Part")
	FString PartKey = "SET ME";
	UPROPERTY(EditDefaultsOnly, Category="Robot Part")
	float MaxHealth = 250.0f;
	UPROPERTY(EditDefaultsOnly, Category="Robot Part")
	float HealthDamageModifier = 1.0f;
	UPROPERTY(EditDefaultsOnly, Category="Robot Part")
	float PersistentDamageChanceModifer = 1.0f;
	UPROPERTY(EditDefaultsOnly, Category="Robot Part")
	float PersistentDamageEnergyDrain = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category="Robot Part")
	TSubclassOf<AActor> PersistentDamageEffectType;

	UPROPERTY(EditDefaultsOnly, Category="Robot Part")
	FString ProxyFor; // TODO: Do this better.

	bool PersistentDamage;

	ABaseRobot* Robot;
	float Health;
	
	AActor* PersistentDamageEffect;
	UPrimitiveComponent* ViewComponent; // TODO: Inherit this class from static mesh instead?
	URobotPartCollider* ProxiedPart;

public:
	URobotPartCollider();
	virtual void TickComponent(float DeltaTime, ELevelTick Type, FActorComponentTickFunction* Self) override;

protected:
	virtual void BeginPlay() override;

public:
	void BindToRobot(ABaseRobot* TargetRobot);

	float ServerRegisterDamageAsPrimary(FDamageProfile Damage); // Returns overflow.
	void ServerRegisterDamageAsOverflow(float Damage);
	void ServerRegisterHeal(float Amount);
	void ServerRegisterHealPersistentDamage();

	void ClientUpdateHealth(float NewHealth, bool HasPersistentDamage);

	float GetCurrentEnergyDrain();

	ABaseRobot* GetRobot() { return this->Robot; }
	
	UFUNCTION(BlueprintCallable)
	bool HasPersistentDamage() { return this->PersistentDamage; }
	float GetMaxHealth() { return this->MaxHealth; }
	float GetHealth() { return this->Health; }
	FString GetPartKey() { return this->PartKey; }
	bool IsProxy() { return IsValid(this->ProxiedPart); }

};
