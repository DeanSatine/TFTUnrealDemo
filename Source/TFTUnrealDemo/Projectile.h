#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent; 
class AUnitBase;

UCLASS()
class TFTUNREALDEMO_API AProjectile : public AActor
{
    GENERATED_BODY()

public:
    AProjectile();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UProjectileMovementComponent* ProjectileMovement;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* ProjectileVFX;  

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    float Damage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    AUnitBase* TargetUnit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    AUnitBase* OwnerUnit;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        FVector NormalImpulse, const FHitResult& Hit);

private:
    void HomingUpdate(float DeltaTime);
};