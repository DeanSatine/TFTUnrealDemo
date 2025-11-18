#include "UnitBase.h"
#include "AIController.h"
#include "WBP_UnitHealthBar.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/WidgetComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Projectile.h"

AUnitBase::AUnitBase()
{
    PrimaryActorTick.bCanEverTick = true;

    HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
    HealthBarWidget->SetupAttachment(RootComponent);
    HealthBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
    HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
    HealthBarWidget->SetDrawSize(FVector2D(150.0f, 30.0f));

    HitImpactVFX = nullptr;
    ProjectileClass = nullptr;
    ProjectileSocketName = FName("Muzzle");
    bIsRangedUnit = false;

    UnitName = TEXT("Unit");
    StarLevel = 1;
    Team = ETeam::Player;
    TeamID = 0;

    MaxHealth = 100.0f;
    CurrentHealth = 100.0f;
    AttackDamage = 10.0f;
    AbilityPower = 100.0f;  
    AttackSpeed = 1.0f;
    AttackRange = 150.0f;
    Armor = 0.0f;
    MagicResist = 0.0f;
    MaxMana = 50.0f;
    CurrentMana = 0.0f;

    CurrentShield = 0.0f;  
    MaxShield = 0.0f;      
    bHasShield = false;    
    ShieldDuration = 0.0f;

    bIsAlive = true;
    bCanMove = true;
    bCanAttack = true;
    bIsCastingAbility = false;
    CurrentTarget = nullptr;

    AttackMontage = nullptr;
    AbilityMontage = nullptr;
    DeathMontage = nullptr;

    MovementSpeed = 300.0f;
    StoppingDistance = 50.0f;

    CurrentState = EUnitState::Bench;
    AttackCooldown = 0.0f;
    AIControllerRef = nullptr;

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;

    GetCapsuleComponent()->SetSimulatePhysics(false);
}

void AUnitBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    CurrentMana = 0.0f;
    AttackCooldown = 0.0f;

    AIControllerRef = Cast<AAIController>(GetController());

   
    if (HealthBarWidget)
    {
        FTimerHandle WidgetInitTimer;
        GetWorld()->GetTimerManager().SetTimer(WidgetInitTimer, [this]()
        {
            if (HealthBarWidget)
            {
                UWBP_UnitHealthBar* Widget = Cast<UWBP_UnitHealthBar>(HealthBarWidget->GetWidget());
                if (Widget)
                {
                    Widget->SetOwningUnit(this);
                }
            }
        }, 0.1f, false);
    }
  

    SetState(EUnitState::Combat);
}

void AUnitBase::UpdateHealthBarWidget()
{

  
    if (HealthBarWidget)
    {
        UWBP_UnitHealthBar* Widget = Cast<UWBP_UnitHealthBar>(HealthBarWidget->GetWidget());
        if (Widget)
        {
            Widget->UpdateBars();
        }
    }
 
}

void AUnitBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsAlive || CurrentState != EUnitState::Combat || bIsCastingAbility)
    {
        return;
    }

    if (AttackCooldown > 0.0f)
    {
        AttackCooldown -= DeltaTime;
    }

    if (CurrentTarget)
    {
        FaceTarget(CurrentTarget->GetActorLocation());
    }

    Think();
}

void AUnitBase::Think()
{
    if (CurrentHealth <= 0.0f)
    {
        return;
    }

    if (!CurrentTarget || !CurrentTarget->bIsAlive || CurrentTarget->CurrentState == EUnitState::Bench)
    {
        FindNewTarget();
        return;
    }

    float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    if (DistanceToTarget > AttackRange)
    {
        MoveToTarget();
        return;
    }

    StopMovement();

    if (CurrentMana >= MaxMana)
    {
        CastAbility();
    }

    if (AttackCooldown <= 0.0f && bCanAttack)
    {
        AttemptAutoAttack();
    }
}

void AUnitBase::FindNewTarget()
{
    CurrentTarget = GetNearestEnemy();

    if (CurrentTarget)
    {
        UE_LOG(LogTemp, Log, TEXT("🎯 %s found new target: %s"), *UnitName, *CurrentTarget->UnitName);
        AttackCooldown = 0.0f;
    }
}
void AUnitBase::ApplyShield(float ShieldAmount, float Duration)
{
    CurrentShield = ShieldAmount;
    MaxShield = ShieldAmount;
    bHasShield = true;
    ShieldDuration = Duration;

    UE_LOG(LogTemp, Log, TEXT("🛡️ %s gained %.0f shield for %.1fs"), *UnitName, ShieldAmount, Duration);

    // Timer to remove shield after duration
    FTimerHandle ShieldTimer;
    GetWorld()->GetTimerManager().SetTimer(ShieldTimer, [this]()
        {
            RemoveShield();
        }, Duration, false);
}

void AUnitBase::RemoveShield()
{
    CurrentShield = 0.0f;
    MaxShield = 0.0f;
    bHasShield = false;

    UE_LOG(LogTemp, Log, TEXT("🛡️ %s shield expired"), *UnitName);
}

AUnitBase* AUnitBase::GetNearestAlly()
{
    TArray<AActor*> AllUnits;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUnitBase::StaticClass(), AllUnits);

    AUnitBase* NearestAlly = nullptr;
    float BestDistance = FLT_MAX;

    for (AActor* Actor : AllUnits)
    {
        AUnitBase* Unit = Cast<AUnitBase>(Actor);

        if (!Unit || Unit == this) continue;
        if (!Unit->bIsAlive) continue;
        if (Unit->Team != this->Team) continue; // ✅ Same team
        if (Unit->CurrentState != EUnitState::Combat) continue;

        float Distance = FVector::Dist(GetActorLocation(), Unit->GetActorLocation());

        if (Distance < BestDistance)
        {
            BestDistance = Distance;
            NearestAlly = Unit;
        }
    }

    return NearestAlly;
}

// ✅ Change CastAbility to be virtual with _Implementation
void AUnitBase::CastAbility_Implementation()
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

    PlayUnitAnimMontage(AbilityMontage);

    UE_LOG(LogTemp, Log, TEXT("🔮 %s casting base ability!"), *UnitName);

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            bIsCastingAbility = false;
        }, 1.5f, false);
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

void AUnitBase::AttemptAutoAttack()
{
    if (!CurrentTarget || !CurrentTarget->bIsAlive)
    {
        return;
    }

    if (CurrentTarget->CurrentState == EUnitState::Bench)
    {
        return;
    }

    FaceTarget(CurrentTarget->GetActorLocation());
    PlayUnitAnimMontage(AttackMontage);

    // ✅ RANGED: Spawn projectile
    if (bIsRangedUnit && ProjectileClass)
    {
        FVector SpawnLocation;

        // Try to get socket location
        if (GetMesh()->DoesSocketExist(ProjectileSocketName))
        {
            SpawnLocation = GetMesh()->GetSocketLocation(ProjectileSocketName);
        }
        else
        {
            // Fallback to actor location + offset
            SpawnLocation = GetActorLocation() + GetActorForwardVector() * 50.0f + FVector(0, 0, 50.0f);
        }

        FRotator SpawnRotation = (CurrentTarget->GetActorLocation() - SpawnLocation).Rotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();

        AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(
            ProjectileClass,
            SpawnLocation,
            SpawnRotation,
            SpawnParams
        );

        if (Projectile)
        {
            Projectile->Damage = AttackDamage;
            Projectile->TargetUnit = CurrentTarget;
            Projectile->OwnerUnit = this;

            UE_LOG(LogTemp, Log, TEXT("🏹 %s fired projectile at %s"), *UnitName, *CurrentTarget->UnitName);
        }
    }
    else
    {
        // ✅ MELEE: Deal damage instantly
        DealDamage(CurrentTarget, AttackDamage, EDamageType::Physical);
    }

    GainMana(10.0f);
    OnAttack.Broadcast(CurrentTarget);
    AttackCooldown = 1.0f / AttackSpeed;

    UE_LOG(LogTemp, Log, TEXT("⚔️ %s attacked %s"), *UnitName, *CurrentTarget->UnitName);
}

void AUnitBase::DealDamage(AUnitBase* Target, float Damage, EDamageType DamageType)
{
    if (!Target || !Target->bIsAlive)
    {
        return;
    }

    FDamageInfo DamageInfo(Damage, DamageType, this);
    Target->ApplyDamage(DamageInfo); 
}

void AUnitBase::ApplyDamage(const FDamageInfo& DamageInfo)
{
    if (CurrentState == EUnitState::Bench)
    {
        return;
    }

    float FinalDamage = CalculateDamageReduction(DamageInfo.Amount, DamageInfo.Type);
    CurrentHealth -= FinalDamage;

    if (FinalDamage > 0.0f)
    {
        GainMana(1.0f);

        if (HitImpactVFX)
        {
            FVector ImpactLocation = GetActorLocation() + FVector(0, 0, 50.0f);

            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                HitImpactVFX,
                ImpactLocation,
                FRotator::ZeroRotator,
                FVector(1.0f),  
                true,           
                true,          
                ENCPoolMethod::None,
                true            
            );
        }
    }

    UpdateHealthBarWidget();

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

void AUnitBase::GainMana(float Amount)
{
    CurrentMana += Amount;
    UpdateHealthBarWidget();

    UE_LOG(LogTemp, Log, TEXT("%s gained %.1f mana → %.1f/%.1f"), *UnitName, Amount, CurrentMana, MaxMana);

    if (CurrentMana >= MaxMana)
    {
        CastAbility();
        CurrentMana = 0.0f;
    }
}

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

void AUnitBase::PlayUnitAnimMontage(UAnimMontage* Montage)
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
        break;

    case EUnitState::BoardIdle:
        bCanMove = true;
        bCanAttack = true;
        AttackCooldown = 0.0f;
        break;

    case EUnitState::Combat:
        CurrentTarget = GetNearestEnemy();
        AttackCooldown = 0.0f;
        bCanMove = true;
        bCanAttack = true;
        break;
    }
}

void AUnitBase::Die()
{
    if (!bIsAlive)
    {
        return;
    }

    bIsAlive = false;
    OnUnitDeath.Broadcast(this);
    StopMovement();
    CurrentTarget = nullptr;
    PlayUnitAnimMontage(DeathMontage);  
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (Team == ETeam::Player)
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
            {
                SetActorHiddenInGame(true);
                SetActorEnableCollision(false);
            }, 1.5f, false);
    }
    else
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
            {
                Destroy();
            }, 2.0f, false);
    }
}

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
}

void AUnitBase::FullResetToPrep()
{
    ResetAfterCombat();
    SetState(EUnitState::Bench);
}