#pragma once

#include "DrawDebugHelpers.h"
#include "GameFramework/GameModeBase.h"

DECLARE_LOG_CATEGORY_EXTERN(AA_Sanity, Log, All);

// TODO: Improve.
#define SANITY_CHECKS 1

FString _UtilGetNetModeName();
int _UtilGetNetId();

#define SANITY_ERR(M) FString _M = TEXT(M); UE_LOG(AA_Sanity, Error, TEXT("%s %d: %s"), *_UtilGetNetModeName(), _UtilGetNetId(), *_M);
#define SANITY_WARN(M) FString _M = TEXT(M); UE_LOG(AA_Sanity, Warning, TEXT("%s %d: %s"), *_UtilGetNetModeName(), _UtilGetNetId(), *_M);

#define SANITY_LOG_D(M, ...) UE_LOG(AA_Sanity, Log, TEXT("%s %d: %s"), *_UtilGetNetModeName(), _UtilGetNetId(), *FString::Printf(TEXT(M), __VA_ARGS__));
#define SANITY_WARN_D(M, ...) UE_LOG(AA_Sanity, Warning, TEXT("%s %d: %s"), *_UtilGetNetModeName(), _UtilGetNetId(), *FString::Printf(TEXT(M), __VA_ARGS__));
#define SANITY_ERR_D(M, ...) UE_LOG(AA_Sanity, Error, TEXT("%s %d: %s"), *_UtilGetNetModeName(), _UtilGetNetId(), *FString::Printf(TEXT(M), __VA_ARGS__));

#define SANITY_WARN_IF(C, M) if (C) { SANITY_WARN(M); }

#define ASSERT_PTR_SANITY(O, M) IsValid(O)) { SANITY_ERR("Non-sane pointer: " M); } if (!IsValid(O)
#define CHECK_BIND_PTR_SANITY(O, M) if (!IsValid(O)) { SANITY_ERR("Non-sane pointer: " M); }

// TODO: Remove.
#define DEBUG_FLASH_S(M) GEngine->AddOnScreenDebugMessage(-1, 0.25f, FColor::Green, TEXT(M));
#define DEBUG_FLASH(M) GEngine->AddOnScreenDebugMessage(-1, 0.25f, FColor::Green, M);

#define DEBUG_LOG_S(M) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT(M));
#define DEBUG_LOG_F(M) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::SanitizeFloat(M));
#define DEBUG_LOG_I(M) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::FromInt(M));
#define DEBUG_LOG(M) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, M);

#define DEBUG_LOG_2_S(M) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT(M));
#define DEBUG_LOG_2_F(M) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::SanitizeFloat(M));
#define DEBUG_LOG_2_I(M) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::FromInt(M));
#define DEBUG_LOG_2(M) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, M);

#define DEBUG_SPHERE(W, L) DrawDebugSphere(W, L, 5.0f, 5, FColor::Green, false, 0.2f, 100);
#define DEBUG_SPHERE_2(W, L) DrawDebugSphere(W, L, 5.0f, 5, FColor::Blue, false, 0.2f, 100);
