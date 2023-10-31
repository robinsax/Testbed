#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Lerp.generated.h"

USTRUCT()
struct FLerped {
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Lerp")
	float LerpRate = 10.0f;

};

// Template USTRUCTs not supported so I guess this is how we roll...
#define LERPED_MEMBERS(T) \
private: \
	T Value; \
	T BaseValue; \
	T TargetValue; \
public: \
	void SetBaseValue(T NewBaseValue); \
	void SetTargetValue(T NewTargetValue); \
	void SetValueHard(T NewValue); \
	void SetRelativeTargetValue(T NewTargetValue); \
	void SetRelativeValueHard(T NewValue); \
	void ResetTargetValue(); \
	void ResetValueHard(); \
	T GetValue(); \
	T GetBaseValue(); \
	T Update(float DeltaTime);

USTRUCT(BlueprintType)
struct FLerpedFloat : public FLerped {
	GENERATED_BODY()
	LERPED_MEMBERS(float)
};

USTRUCT(BlueprintType)
struct FLerpedVector : public FLerped {
	GENERATED_BODY()
	LERPED_MEMBERS(FVector)
};

USTRUCT(BlueprintType)
struct FLerpedRotator : public FLerped {
	GENERATED_BODY()
	LERPED_MEMBERS(FRotator)
};
