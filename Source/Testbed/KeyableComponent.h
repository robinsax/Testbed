#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "KeyableComponent.generated.h"

UINTERFACE(MinimalAPI)
class UKeyableComponent : public UInterface {
	GENERATED_BODY()
};

class IKeyableComponent {
	GENERATED_BODY()

public:
	virtual FString GetKey() { return TEXT(""); };

};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UAnchorComponent : public USceneComponent, public IKeyableComponent {
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category="Anchor")
	FString Key;

public:
	static UAnchorComponent* FindOne(AActor* Actor, FString Key);

	virtual FString GetKey() { return this->Key; }

};
