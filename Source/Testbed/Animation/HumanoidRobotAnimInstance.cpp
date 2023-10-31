#include "HumanoidRobotAnimInstance.h"

#include "Kismet/KismetMathLibrary.h"

#include "../Util.h"
#include "../KeyableComponent.h"
#include "../Robots/HumanoidRobot.h"
#include "../Items/ItemActor.h"
#include "../Items/InventorySlotComponent.h"

void UHumanoidRobotAnimInstance::BindToRobot(ABaseRobot* TargetRobot) {
    Super::BindToRobot(TargetRobot);

    this->Robot = Cast<AHumanoidRobot>(TargetRobot);

    this->Step = -1;
    this->StepTime = -1.0f;
    this->StepAlpha = 0.0f;
    this->StepTarget = FVector::ZeroVector;

    this->TargetLegEffectors = TArray<FVector>();
    for (int I = 0; I < this->Robot->LegOffsets.Num(); I++) {
        this->TargetLegEffectors.Push(FVector::ZeroVector);
        this->LegEffectors.Push(FVector::ZeroVector);
    }

    this->TargetArmEffectors = TArray<FVector>();
    for (int I = 0; I < this->Robot->ArmOffsets.Num(); I++) {
        this->TargetArmEffectors.Push(FVector::ZeroVector);
        this->LegEffectors.Push(FVector::ZeroVector);
        this->ArmEffectorTicksSinceAnimate.Push(0);
    }

    this->WorldLookRotationLerp.SetBaseValue(FRotator());
    this->TorsoYawLerp.SetBaseValue(0.0f);
}

void UHumanoidRobotAnimInstance::Tick(float DeltaTime) {
    this->CurrentAnimationEffectorOverrides = TArray<FString>();

    AItemActor* EquipedItem = this->Robot->GetEquippedItem();
    bool InInventory = this->Robot->IsInInventory();
    
    if (!InInventory && IsValid(EquipedItem)) {
        this->TorsoYawLerp.SetTargetValue(EquipedItem->GetEquipModifiers().RootYaw);
    }
    else {
        this->TorsoYawLerp.ResetTargetValue();
    }

    this->TorsoYaw = this->TorsoYawLerp.Update(DeltaTime);

    // Blueprint-exposed properties.
    UCharacterMovementComponent* RobotMovement = this->Robot->GetCharacterMovement();
    this->Velocity = RobotMovement->GetLastUpdateVelocity();
    this->Falling = RobotMovement->IsFalling();
    this->Rotation = this->Robot->GetActorRotation();
    this->Rotation.Yaw += this->TorsoYaw;
    this->Crouching = this->Robot->IsCrouching();
    this->Lean = this->Robot->GetLean();

    // Look targeting.
    UAnchorComponent* Eye = this->Robot->GetEye();
    if (Robot->IsCheckingStatus()) {
        // TODO: Messy.
        FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(Eye->GetComponentLocation(), UAnchorComponent::FindOne(this->Robot, TEXT("CheckStatus"))->GetComponentLocation() + FVector(0.0f, 0.0f, 30.0f));
        LookRotation.Yaw -= this->Rotation.Yaw - this->TorsoYaw;

        this->WorldLookRotationLerp.SetTargetValue(LookRotation);
    }
    else if (!InInventory) {
        FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(Eye->GetComponentLocation(), this->Robot->GetWorldLookLocation());
        LookRotation.Yaw -= this->Rotation.Yaw - this->TorsoYaw;

        this->WorldLookRotationLerp.SetTargetValue(LookRotation);
    }
    else {
        this->WorldLookRotationLerp.SetTargetValue(FRotator());
    }
    
    this->WorldLookRotation = this->WorldLookRotationLerp.Update(DeltaTime);

    // Leg IK.
    this->TickLegIK(DeltaTime);

    // Animation.
    Super::Tick(DeltaTime);

    // Default arm IK.
    if (!this->CurrentAnimationEffectorOverrides.Contains(TEXT("LeftHand"))) {
        if (this->ArmEffectorTicksSinceAnimate[0] > 4) {
            this->TargetArmEffectors[0] = this->Robot->ArmOffsets[0];
        }
        else {
            this->ArmEffectorTicksSinceAnimate[0]++;
        }
    }
    else {
        this->ArmEffectorTicksSinceAnimate[0] = 0;
    }
    if (!this->CurrentAnimationEffectorOverrides.Contains(TEXT("RightHand"))) {
        if (this->ArmEffectorTicksSinceAnimate[1] > 4) {
            this->TargetArmEffectors[1] = this->Robot->ArmOffsets[1];
        }
        else {
            this->ArmEffectorTicksSinceAnimate[1]++;
        }
    }
    else {
        this->ArmEffectorTicksSinceAnimate[1] = 0;
    }

    //#define SHOW_EFFECTORS
    #ifdef SHOW_EFFECTORS
    for (int I = 0; I < this->TargetLegEffectors.Num(); I++) {
        DEBUG_SPHERE(this->GetWorld(), this->TargetLegEffectors[I]);
        DEBUG_SPHERE_2(this->GetWorld(), this->LegEffectors[I]);
    }
    for (int I = 0; I < this->TargetArmEffectors.Num(); I++) {
        DEBUG_SPHERE(this->GetWorld(), this->GetSkelMeshComponent()->GetComponentTransform().TransformPosition(this->TargetArmEffectors[I]));
        DEBUG_SPHERE_2(this->GetWorld(), this->GetSkelMeshComponent()->GetComponentTransform().TransformPosition(this->ArmEffectors[I]))
    }
    #endif

    // Smoothing.
    float LegInterpSpeed = 25.0f;
    float ArmInterpSpeed = 15.0f;
    for (int I = 0; I < this->TargetLegEffectors.Num(); I++) {
        FVector Target = this->TargetLegEffectors[I];
        if (this->LegEffectors[I].IsZero()) {
            this->LegEffectors.Push(Target);
        }
        else {
            this->LegEffectors[I] = FMath::Lerp(this->LegEffectors[I], Target, LegInterpSpeed * DeltaTime);
        }
    }
    for (int I = 0; I < this->TargetArmEffectors.Num(); I++) {
        FVector Target = this->TargetArmEffectors[I];
        if (this->ArmEffectors[I].IsZero()) {
            this->ArmEffectors.Push(Target);
        }
        else {
            this->ArmEffectors[I] = FMath::Lerp(this->ArmEffectors[I], Target, ArmInterpSpeed * DeltaTime);
        }
    }
}

void UHumanoidRobotAnimInstance::TickLegIK(float DeltaTime) {
    FVector BaseLocation = this->Robot->GetActorLocation();
    FVector EffectiveVelocity = this->Velocity * this->Robot->StepVelocityScale;
    float EffectiveStepVertical = this->Robot->StepVerticalHeight;

    if (this->Robot->IsCrouching()) {
        EffectiveVelocity *= this->Robot->CrouchStepScale;
        EffectiveStepVertical *= this->Robot->CrouchStepScale;
    }

    if (this->Falling) {
        for (int I = 0; I < this->TargetLegEffectors.Num(); I++) {
            this->TargetLegEffectors[I] = BaseLocation + this->Rotation.RotateVector(this->Robot->LegOffsets[I] + FVector(2.0, 0.0, -this->Robot->JumpLegOffset));

            if (!this->MustStep.Contains(I)) this->MustStep.Push(I);
        }
        return;
    }

    if (this->Step < 0) {
        // Choose next step.
        TArray<FVector> ExactTargets;
        for (int I = 0; I < this->TargetLegEffectors.Num(); I++) {
            ExactTargets.Push(BaseLocation + EffectiveVelocity + this->Rotation.RotateVector(this->Robot->LegOffsets[I]));
        }

        // TODO: Clean.
        int BestStep = -1;
        if (this->MustStep.Num() > 0) BestStep = this->MustStep.Pop();
        else {
            float MaxOffset = this->Robot->StepOffsetTolerance;
            for (int I = 0; I < this->TargetLegEffectors.Num(); I++) {
                FVector Check = this->TargetLegEffectors[I];

                FVector Delta = Check - ExactTargets[I];
                Delta.Z = 0.0; 
                float Offset = Delta.Size();
                if (Offset > MaxOffset) {
                    MaxOffset = Offset;
                    BestStep = I;
                }
            }
        }

        if (BestStep >= 0) {
            this->Step = BestStep;
            this->StepTime = 0;
            this->StepTarget = ExactTargets[BestStep];
            
            this->CurrentStepDuration = this->Robot->StepDuration;
        }
    }
    
    if (this->StepTime > this->CurrentStepDuration) {
        this->Step = -1;
        this->StepAlpha = 0.0f;
    }
    if (this->Step >= 0) {
        this->StepAlpha = this->StepTime / this->CurrentStepDuration;
        this->StepTime += DeltaTime;

        FVector CurrentEffector = FVector(this->StepTarget);
        
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this->Robot);
        
        TArray<AActor*> AttachedActors;
        this->Robot->GetAttachedActors(AttachedActors);
        for (int I = 0; I < AttachedActors.Num(); I++) {
            QueryParams.AddIgnoredActor(AttachedActors[I]);
        }

        FHitResult HitResult;
        this->GetWorld()->LineTraceSingleByChannel(HitResult, BaseLocation, BaseLocation + FVector(0, 0, -200), ECC_GameTraceChannel7, QueryParams);

        CurrentEffector.Z = HitResult.ImpactPoint.Z + (FMath::Sin(this->StepAlpha * 3.14159) * EffectiveStepVertical);

        this->TargetLegEffectors[this->Step] = CurrentEffector;
    }
}

void UHumanoidRobotAnimInstance::ApplyAnimationMove(FString TargetName, FVector Location, USceneComponent* Anchor) {
    if (TargetName.Equals(TEXT("LeftHand"))) {
        this->TargetArmEffectors[0] = Location;
    }
    else if (TargetName.Equals(TEXT("RightHand"))) {
        this->TargetArmEffectors[1] = Location;

        AItemActor* EquippedItem = this->Robot->GetEquippedItem();
        if (IsValid(EquippedItem)) {
            EquippedItem->SetTargetRotation(Anchor->GetComponentRotation());
        }
    }
    else {
        SANITY_WARN_D("Unknown move target %s", *TargetName);
        return;
    }

    this->CurrentAnimationEffectorOverrides.Push(TargetName);
}
