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
    TrueDamage UMETA(DisplayName = "True Damage")  // Changed from "True" to "TrueDamage"
};

// ============================================================================
// STRUCTS
// ============================================================================

USTRUCT(BlueprintType)
struct FDamageInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float Amount = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    EDamageType Type = EDamageType::Physical;

    UPROPERTY(BlueprintReadWrite)
    AActor* Causer = nullptr;

    FDamageInfo() { }

    FDamageInfo(float InAmount, EDamageType InType, AActor* InCauser = nullptr)
        : Amount(InAmount), Type(InType), Causer(InCauser) { }
};

// ============================================================================
// MAIN UNIT BASE CLASS
// ============================================================================

UCLASS()
class TFTUNREALDEMO_API AUnitBase: public ACharacter
{
    GENERATED_BODY()

public:
    AUnitBase();

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Info")
    FString UnitName;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Info")
    ETeam Team = ETeam::Player;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float CurrentHealth;

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
};