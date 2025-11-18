#include "Projectile.h"
#include "UnitBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectile::AProjectile()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->InitSphereRadius(15.0f);
    CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
    RootComponent = CollisionSphere;

    // Create projectile movement
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionSphere;
    ProjectileMovement->InitialSpeed = 1000.0f;
    ProjectileMovement->MaxSpeed = 1000.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f;

    ProjectileVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileVFX"));
    ProjectileVFX->SetupAttachment(RootComponent);

    Damage = 10.0f;
    TargetUnit = nullptr;
    OwnerUnit = nullptr;

    // Set lifetime
    InitialLifeSpan = 5.0f;
}

void AProjectile::BeginPlay()
{
    Super::BeginPlay();

    CollisionSphere->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
}

void AProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Homing behavior
    if (TargetUnit && TargetUnit->bIsAlive)
    {
        HomingUpdate(DeltaTime);
    }
}

void AProjectile::HomingUpdate(float DeltaTime)
{
    if (!TargetUnit || !TargetUnit->bIsAlive)
    {
        return;
    }

    FVector TargetLocation = TargetUnit->GetActorLocation();
    FVector CurrentLocation = GetActorLocation();
    FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();

    // Smooth homing
    FVector CurrentVelocity = ProjectileMovement->Velocity;
    FVector NewVelocity = FMath::Lerp(CurrentVelocity, Direction * ProjectileMovement->MaxSpeed, DeltaTime * 5.0f);

    ProjectileMovement->Velocity = NewVelocity;
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    FVector NormalImpulse, const FHitResult& Hit)
{
    AUnitBase* HitUnit = Cast<AUnitBase>(OtherActor);

    // Only damage the target unit, not the owner
    if (HitUnit && HitUnit == TargetUnit && HitUnit != OwnerUnit)
    {
        // Deal damage
        if (OwnerUnit)
        {
            OwnerUnit->DealDamage(HitUnit, Damage, EDamageType::Physical);
        }

        UE_LOG(LogTemp, Log, TEXT("💥 Projectile hit %s!"), *HitUnit->UnitName);
    }

    // Destroy projectile
    Destroy();
}