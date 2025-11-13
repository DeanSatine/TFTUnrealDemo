// UnitBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UnitBase.generated.h"

UCLASS()
class TFTUNREALDEMO_API AUnitBase : public ACharacter
{
    GENERATED_BODY()

public:
    AUnitBase();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Info")
    FString UnitName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float CurrentHealth;

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
};