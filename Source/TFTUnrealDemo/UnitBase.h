// UnitBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UnitBase.generated.h"

// Forward declarations
class UAnimMontage;
class AAIController;

// ============================================================================
// ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class ETeam : uint8
{
    Player UMETA(DisplayName = "Player"),
    Enemy UMETA(DisplayName = "Enemy"),
    Neutral UMETA(DisplayName = "Neutral")
};

UENUM(BlueprintType)
enum class EUnitState : uint8
{
    Bench UMETA(DisplayName = "Bench"),
    BoardIdle UMETA(DisplayName = "Board Idle"),
    Combat UMETA(DisplayName = "Combat")
};

UENUM(BlueprintType)
enum class EDamageType : uint8
{
    Physical UMETA(DisplayName = "Physical"),
    Magical UMETA(DisplayName = "Magical"),
    TrueDamage UMETA(DisplayName = "True Damage")
};

// ============================================================================
// STRUCTS
// ============================================================================

USTRUCT(BlueprintType)
struct FDamageInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float Amount;

    UPROPERTY(BlueprintReadWrite)
    EDamageType Type;

    UPROPERTY(BlueprintReadWrite)
    AActor* Causer;

    FDamageInfo()
        : Amount(0.0f), Type(EDamageType::Physical), Causer(nullptr)
    {
    }

    FDamageInfo(float InAmount, EDamageType InType, AActor* InCauser = nullptr)
        : Amount(InAmount), Type(InType), Causer(InCauser)
    {
    }
};

// ============================================================================
// MAIN UNIT BASE CLASS
// ============================================================================

UCLASS()
class TFTUNREALDEMO_API AUnitBase : public ACharacter
{
    GENERATED_BODY()

public:
    AUnitBase();

    // ========================================================================
    // PROPERTIES - Unit Info
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Info")
    FString UnitName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Info")
    int32 StarLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Info")
    ETeam Team;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Info")
    int32 TeamID;

    // ========================================================================
    // PROPERTIES - Stats
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Armor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MagicResist;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Mana")
    float MaxMana;

    UPROPERTY(BlueprintReadOnly, Category = "Stats|Mana")
    float CurrentMana;

    // ========================================================================
    // PROPERTIES - Combat State
    // ========================================================================

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    bool bIsAlive;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    bool bCanMove;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    bool bCanAttack;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    bool bIsCastingAbility;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AUnitBase* CurrentTarget;

    // ========================================================================
    // PROPERTIES - Animation
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* AbilityMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* DeathMontage;

    // ========================================================================
    // PROPERTIES - Movement
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float StoppingDistance;

    // ========================================================================
    // PUBLIC METHODS - AI
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "AI")
    void Think();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void FindNewTarget();

    UFUNCTION(BlueprintCallable, Category = "AI")
    AUnitBase* GetNearestEnemy();

    // ========================================================================
    // PUBLIC METHODS - Combat
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttemptAutoAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void DealDamage(AUnitBase* Target, float Damage, EDamageType DamageType);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(const FDamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void GainMana(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual void CastAbility();

    // ========================================================================
    // PUBLIC METHODS - Movement
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveToTarget();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopMovement();

    // ========================================================================
    // PUBLIC METHODS - State Management
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "State")
    void SetState(EUnitState NewState);

    UFUNCTION(BlueprintPure, Category = "State")
    EUnitState GetState() const { return CurrentState; }

    // ========================================================================
    // PUBLIC METHODS - Reset Functions
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Reset")
    void ResetAfterCombat();

    UFUNCTION(BlueprintCallable, Category = "Reset")
    void FullResetToPrep();

    // ========================================================================
    // PUBLIC METHODS - Death
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Die();

    // ========================================================================
    // EVENTS (Delegates)
    // ========================================================================

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitDeath, AUnitBase*, DeadUnit);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnUnitDeath OnUnitDeath;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChanged, EUnitState, NewState);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnStateChanged OnStateChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttack, AUnitBase*, Target);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAttack OnAttack;

protected:
    virtual void BeginPlay() override;

    void FaceTarget(const FVector& TargetLocation);
    float CalculateDamageReduction(float IncomingDamage, EDamageType DamageType) const;
    void PlayAnimMontage(UAnimMontage* Montage);

private:
    EUnitState CurrentState;
    float AttackCooldown;
    AAIController* AIControllerRef;

public:
    virtual void Tick(float DeltaTime) override;
};