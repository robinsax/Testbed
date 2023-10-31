#include "Lerp.h"

#include "Util.h"

// Guh.
#define LERPED_IMPL(S, T) \
void S::SetBaseValue(T NewBaseValue) { \
    this->BaseValue = NewBaseValue; \
    this->Value = this->BaseValue; \
    this->TargetValue = this->BaseValue; \
} \
void S::SetTargetValue(T NewTargetValue) { \
    this->TargetValue = NewTargetValue; \
} \
void S::SetValueHard(T NewValue) { \
    this->Value = NewValue; \
    this->TargetValue = this->Value; \
} \
void S::SetRelativeTargetValue(T NewTargetValue) { \
    this->TargetValue = this->BaseValue + NewTargetValue; \
} \
void S::SetRelativeValueHard(T NewValue) { \
    this->SetValueHard(this->BaseValue + NewValue); \
} \
void S::ResetTargetValue() { \
    this->TargetValue = this->BaseValue; \
} \
void S::ResetValueHard() { \
    this->SetValueHard(this->BaseValue); \
} \
T S::GetValue() { \
    return this->Value; \
} \
T S::GetBaseValue() { \
    return this->BaseValue; \
} \
T S::Update(float DeltaTime) { \
    return this->Value = FMath::Lerp(this->Value, this->TargetValue, this->LerpRate * DeltaTime); \
}

LERPED_IMPL(FLerpedFloat, float)
LERPED_IMPL(FLerpedRotator, FRotator)
LERPED_IMPL(FLerpedVector, FVector)
