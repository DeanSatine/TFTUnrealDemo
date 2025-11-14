// UnitBase.cpp

#include "UnitBase.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================================
// CONSTRUCTOR
// ============================================================================

AUnitBase::AUnitBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize default values
    UnitName = TEXT("Unit");
    StarLevel = 1;
    Team = ETeam::Player;
    TeamID = 0;

    MaxHealth = 100.0f;
    CurrentHealth = 100.0f;
    AttackDamage = 10.0f;
    AttackSpeed = 1.0f;
    AttackRange = 150.0f;
    Armor = 0.0f;
    MagicResist = 0.0f;
    MaxMana = 50.0f;
    CurrentMana = 0.0f;

    bIsAlive = true;
    bCanMove = true;
    bCanAttack = true;
    bIsCastingAbility = false;
    CurrentTarget = nullptr;

    AttackMontage = nullptr;
    AbilityMontage = nullptr;
    DeathMontage = nullptr;

    MovementSpeed = 300.0f;
    StoppingDistance = 140.0f;

    CurrentState = EUnitState::Bench;
    AttackCooldown = 0.0f;
    AIControllerRef = nullptr;

    // Set this character to be controlled by AI
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;

    // Disable physics simulation
    GetCapsuleComponent()->SetSimulatePhysics(false);
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void AUnitBase::BeginPlay()
{
    Super::BeginPlay();

    // Initialize stats
    CurrentHealth = MaxHealth;
    CurrentMana = 0.0f;
    AttackCooldown = 0.0f;

    // Get AI Controller reference
    AIControllerRef = Cast<AAIController>(GetController());

    if (!AIControllerRef)
    {
        UE_LOG(LogTemp, Error, TEXT("%s: No AI Controller found!"), *UnitName);
    }

    UE_LOG(LogTemp, Log, TEXT("✅ %s initialized - HP: %.0f/%.0f, Team: %d"),
        *UnitName, CurrentHealth, MaxHealth, (int32)Team);
}

// ============================================================================
// TICK
// ============================================================================

void AUnitBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Only think during combat
    if (!bIsAlive || CurrentState != EUnitState::Combat || bIsCastingAbility)
    {
        return;
    }

    // Reduce attack cooldown
    if (AttackCooldown > 0.0f)
    {
        AttackCooldown -= DeltaTime;
    }

    // Face target if we have one
    if (CurrentTarget)
    {
        FaceTarget(CurrentTarget->GetActorLocation());
    }

    // Main AI logic
    Think();
}

// ============================================================================
// AI - THINK FUNCTION
// ============================================================================

void AUnitBase::Think()
{
    // 1. Check if dead
    if (CurrentHealth <= 0.0f)
    {
        return;
    }

    // 2. Check if we have a valid target
    if (!CurrentTarget || !CurrentTarget->bIsAlive || CurrentTarget->CurrentState == EUnitState::Bench)
    {
        FindNewTarget();
        return;
    }

    // 3. Check distance to target
    float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    // 4. If too far, move closer
    if (DistanceToTarget > AttackRange)
    {
        MoveToTarget();
        return;
    }

    // 5. If in range, stop and prepare to attack
    StopMovement();

    // 6. Check if we should cast ability (mana full)
    if (CurrentMana >= MaxMana)
    {
        CastAbility();
    }

    // 7. Auto attack on cooldown
    if (AttackCooldown <= 0.0f && bCanAttack)
    {
        AttemptAutoAttack();
    }
}

// ============================================================================
// AI - TARGET FINDING
// ============================================================================

void AUnitBase::FindNewTarget()
{
    CurrentTarget = GetNearestEnemy();

    if (CurrentTarget)
    {
        UE_LOG(LogTemp, Log, TEXT("🎯 %s found new target: %s"), *UnitName, *CurrentTarget->UnitName);
        AttackCooldown = 0.0f;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ %s could not find a target!"), *UnitName);
    }
}

AUnitBase* AUnitBase::GetNearestEnemy()
{
    TArray<AActor*> AllUnits;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUnitBase::StaticClass(), AllUnits);

    AUnitBase* BestTarget = nullptr;
    float BestDistance = FLT_MAX;

    for (AActor* Actor : AllUnits)
    {
        AUnitBase* Unit = Cast<AUnitBase>(Actor);

        if (!Unit || Unit == this) continue;
        if (!Unit->bIsAlive) continue;
        if (Unit->Team == this->Team) continue;
        if (Unit->CurrentState != EUnitState::Combat) continue;

        float Distance = FVector::Dist(GetActorLocation(), Unit->GetActorLocation());

        if (Distance < BestDistance)
        {
            BestDistance = Distance;
            BestTarget = Unit;
        }
    }

    return BestTarget;
}

// ============================================================================
// COMBAT - AUTO ATTACK
// ============================================================================

void AUnitBase::AttemptAutoAttack()
{
    if (!CurrentTarget || !CurrentTarget->bIsAlive)
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠️ %s tried to attack invalid target"), *UnitName);
        return;
    }

    if (CurrentTarget->CurrentState == EUnitState::Bench)
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠️ %s tried to attack benched target"), *UnitName);
        return;
    }

    FaceTarget(CurrentTarget->GetActorLocation());
    PlayAnimMontage(AttackMontage);
    DealDamage(CurrentTarget, AttackDamage, EDamageType::Physical);
    GainMana(10.0f);
    OnAttack.Broadcast(CurrentTarget);
    AttackCooldown = 1.0f / AttackSpeed;

    UE_LOG(LogTemp, Log, TEXT("⚔️ %s attacked %s for %.1f damage"), *UnitName, *CurrentTarget->UnitName, AttackDamage);
}

// ============================================================================
// COMBAT - DAMAGE SYSTEM
// ============================================================================

void AUnitBase::DealDamage(AUnitBase* Target, float Damage, EDamageType DamageType)
{
    if (!Target || !Target->bIsAlive)
    {
        return;
    }

    FDamageInfo DamageInfo(Damage, DamageType, this);
    Target->TakeDamage(DamageInfo);
}

void AUnitBase::TakeDamage(const FDamageInfo& DamageInfo)
{
    if (CurrentState == EUnitState::Bench)
    {
        UE_LOG(LogTemp, Log, TEXT("🚫 %s is benched and ignored damage"), *UnitName);
        return;
    }

    float FinalDamage = CalculateDamageReduction(DamageInfo.Amount, DamageInfo.Type);
    CurrentHealth -= FinalDamage;

    if (FinalDamage > 0.0f)
    {
        GainMana(1.0f);
    }

    FString DamageTypeStr = DamageInfo.Type == EDamageType::Physical ? TEXT("Physical") :
        DamageInfo.Type == EDamageType::Magical ? TEXT("Magical") : TEXT("True");

    UE_LOG(LogTemp, Log, TEXT("💥 %s took %.1f %s damage. HP: %.0f/%.0f"),
        *UnitName, FinalDamage, *DamageTypeStr, CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
}

float AUnitBase::CalculateDamageReduction(float IncomingDamage, EDamageType DamageType) const
{
    float FinalDamage = IncomingDamage;

    switch (DamageType)
    {
    case EDamageType::Physical:
        FinalDamage = IncomingDamage * (100.0f / (100.0f + Armor));
        break;

    case EDamageType::Magical:
        FinalDamage = IncomingDamage * (100.0f / (100.0f + MagicResist));
        break;

    case EDamageType::TrueDamage:
        FinalDamage = IncomingDamage;
        break;
    }

    return FinalDamage;
}

// ============================================================================
// COMBAT - MANA SYSTEM
// ============================================================================

void AUnitBase::GainMana(float Amount)
{
    CurrentMana += Amount;

    UE_LOG(LogTemp, Log, TEXT("✨ %s gained %.1f mana → %.1f/%.1f"), *UnitName, Amount, CurrentMana, MaxMana);

    if (CurrentMana >= MaxMana)
    {
        UE_LOG(LogTemp, Log, TEXT("🌟 %s mana full! Casting ability..."), *UnitName);
        CastAbility();
        CurrentMana = 0.0f;
    }
}

void AUnitBase::CastAbility()
{
    if (bIsCastingAbility)
    {
        return;
    }

    bIsCastingAbility = true;

    if (CurrentTarget)
    {
        FaceTarget(CurrentTarget->GetActorLocation());
    }

    PlayAnimMontage(AbilityMontage);

    UE_LOG(LogTemp, Log, TEXT("🔮 %s casting ability!"), *UnitName);

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            bIsCastingAbility = false;
            UE_LOG(LogTemp, Log, TEXT("✅ %s finished casting ability"), *UnitName);
        }, 1.5f, false);
}

// ============================================================================
// MOVEMENT
// ============================================================================

void AUnitBase::MoveToTarget()
{
    if (!bCanMove || !AIControllerRef || !CurrentTarget)
    {
        return;
    }

    AIControllerRef->MoveToActor(CurrentTarget, StoppingDistance);
}

void AUnitBase::StopMovement()
{
    if (AIControllerRef)
    {
        AIControllerRef->StopMovement();
    }

    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->StopMovementImmediately();
    }
}

void AUnitBase::FaceTarget(const FVector& TargetLocation)
{
    FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
    Direction.Z = 0.0f;

    if (!Direction.IsNearlyZero())
    {
        FRotator NewRotation = Direction.Rotation();
        FRotator CurrentRotation = GetActorRotation();
        FRotator SmoothedRotation = FMath::RInterpTo(CurrentRotation, NewRotation, GetWorld()->GetDeltaSeconds(), 10.0f);
        SetActorRotation(SmoothedRotation);
    }
}

// ============================================================================
// ANIMATION
// ============================================================================

void AUnitBase::PlayAnimMontage(UAnimMontage* Montage)
{
    if (!Montage)
    {
        return;
    }

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Play(Montage);
    }
}

// ============================================================================
// STATE MANAGEMENT
// ============================================================================

void AUnitBase::SetState(EUnitState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }

    CurrentState = NewState;
    OnStateChanged.Broadcast(NewState);

    switch (NewState)
    {
    case EUnitState::Bench:
        CurrentTarget = nullptr;
        bCanMove = false;
        bCanAttack = false;
        StopMovement();
        UE_LOG(LogTemp, Log, TEXT("🪑 %s benched"), *UnitName);
        break;

    case EUnitState::BoardIdle:
        bCanMove = true;
        bCanAttack = true;
        AttackCooldown = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("📍 %s placed on board"), *UnitName);
        break;

    case EUnitState::Combat:
        CurrentTarget = GetNearestEnemy();
        AttackCooldown = 0.0f;
        bCanMove = true;
        bCanAttack = true;
        UE_LOG(LogTemp, Log, TEXT("⚔️ %s entered combat!"), *UnitName);
        break;
    }
}

// ============================================================================
// DEATH
// ============================================================================

void AUnitBase::Die()
{
    if (!bIsAlive)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("💀 %s died!"), *UnitName);

    bIsAlive = false;
    OnUnitDeath.Broadcast(this);
    StopMovement();
    CurrentTarget = nullptr;
    PlayAnimMontage(DeathMontage);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (Team == ETeam::Player)
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
            {
                SetActorHiddenInGame(true);
                SetActorEnableCollision(false);
                UE_LOG(LogTemp, Log, TEXT("👻 %s hidden after death"), *UnitName);
            }, 1.5f, false);
    }
    else
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
            {
                UE_LOG(LogTemp, Log, TEXT("🗑️ %s destroyed"), *UnitName);
                Destroy();
            }, 2.0f, false);
    }
}

// ============================================================================
// RESET FUNCTIONS
// ============================================================================

void AUnitBase::ResetAfterCombat()
{
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    bIsAlive = true;
    CurrentHealth = MaxHealth;
    CurrentMana = 0.0f;
    AttackCooldown = 0.0f;
    bIsCastingAbility = false;
    CurrentTarget = nullptr;
    SetState(EUnitState::BoardIdle);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    UE_LOG(LogTemp, Log, TEXT("🔄 %s reset for new round"), *UnitName);
}

void AUnitBase::FullResetToPrep()
{
    ResetAfterCombat();
    SetState(EUnitState::Bench);
    UE_LOG(LogTemp, Log, TEXT("🔄 %s fully reset to prep phase"), *UnitName);
}