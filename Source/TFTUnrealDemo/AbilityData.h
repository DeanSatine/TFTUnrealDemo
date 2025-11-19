#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AbilityData.generated.h"

// Ability type enum
UENUM(BlueprintType)
enum class EAbilityType : uint8
{
    Shield UMETA(DisplayName = "Shield"),
    Damage UMETA(DisplayName = "Damage"),
    Heal UMETA(DisplayName = "Heal"),
    Buff UMETA(DisplayName = "Buff"),
    Summon UMETA(DisplayName = "Summon")
};

UENUM(BlueprintType)
enum class EAbilityTargetType : uint8
{
    Self UMETA(DisplayName = "Self"),
    NearestAlly UMETA(DisplayName = "Nearest Ally"),
    NearestEnemy UMETA(DisplayName = "Nearest Enemy"),
    AllAllies UMETA(DisplayName = "All Allies"),
    AllEnemies UMETA(DisplayName = "All Enemies"),
    CurrentTarget UMETA(DisplayName = "Current Target")
};

/**
 * Data asset that defines an ability's properties
 */
UCLASS(BlueprintType)
class TFTUNREALDEMO_API UAbilityData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // ========================================================================
    // BASIC INFO
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability Info")
    FText AbilityName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability Info")
    FText AbilityDescription;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability Info")
    EAbilityType AbilityType;

    // ========================================================================
    // TARGETING
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
    EAbilityTargetType PrimaryTarget;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
    EAbilityTargetType SecondaryTarget;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
    bool bHasSecondaryTarget;

    // ========================================================================
    // SHIELD VALUES (Star-level based)
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shield", meta = (EditCondition = "AbilityType == EAbilityType::Shield"))
    float BaseShield_Star1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shield", meta = (EditCondition = "AbilityType == EAbilityType::Shield"))
    float BaseShield_Star2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shield", meta = (EditCondition = "AbilityType == EAbilityType::Shield"))
    float BaseShield_Star3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shield", meta = (EditCondition = "AbilityType == EAbilityType::Shield"))
    float APRatio_Star1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shield", meta = (EditCondition = "AbilityType == EAbilityType::Shield"))
    float APRatio_Star2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shield", meta = (EditCondition = "AbilityType == EAbilityType::Shield"))
    float APRatio_Star3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shield", meta = (EditCondition = "AbilityType == EAbilityType::Shield"))
    float ShieldDuration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shield", meta = (EditCondition = "AbilityType == EAbilityType::Shield && bHasSecondaryTarget"))
    float SecondaryTargetMultiplier; // For "half shield to ally"

    // ========================================================================
    // DAMAGE VALUES
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (EditCondition = "AbilityType == EAbilityType::Damage"))
    float BaseDamage_Star1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (EditCondition = "AbilityType == EAbilityType::Damage"))
    float BaseDamage_Star2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (EditCondition = "AbilityType == EAbilityType::Damage"))
    float BaseDamage_Star3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (EditCondition = "AbilityType == EAbilityType::Damage"))
    float DamageAPRatio_Star1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (EditCondition = "AbilityType == EAbilityType::Damage"))
    float DamageAPRatio_Star2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (EditCondition = "AbilityType == EAbilityType::Damage"))
    float DamageAPRatio_Star3;

    // ========================================================================
    // VFX & ANIMATION
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    class UNiagaraSystem* CastVFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    class UNiagaraSystem* ImpactVFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    class UAnimMontage* AbilityMontage;

    // ========================================================================
    // TIMING
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing")
    float CastTime;

    // ========================================================================
    // HELPER FUNCTIONS
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "Ability")
    float GetBaseShield(int32 StarLevel) const
    {
        switch (StarLevel)
        {
        case 1: return BaseShield_Star1;
        case 2: return BaseShield_Star2;
        case 3: return BaseShield_Star3;
        default: return BaseShield_Star1;
        }
    }

    UFUNCTION(BlueprintPure, Category = "Ability")
    float GetAPRatio(int32 StarLevel) const
    {
        switch (StarLevel)
        {
        case 1: return APRatio_Star1;
        case 2: return APRatio_Star2;
        case 3: return APRatio_Star3;
        default: return APRatio_Star1;
        }
    }

    UFUNCTION(BlueprintPure, Category = "Ability")
    float GetBaseDamage(int32 StarLevel) const
    {
        switch (StarLevel)
        {
        case 1: return BaseDamage_Star1;
        case 2: return BaseDamage_Star2;
        case 3: return BaseDamage_Star3;
        default: return BaseDamage_Star1;
        }
    }

    UFUNCTION(BlueprintPure, Category = "Ability")
    float GetDamageAPRatio(int32 StarLevel) const
    {
        switch (StarLevel)
        {
        case 1: return DamageAPRatio_Star1;
        case 2: return DamageAPRatio_Star2;
        case 3: return DamageAPRatio_Star3;
        default: return DamageAPRatio_Star1;
        }
    }
};