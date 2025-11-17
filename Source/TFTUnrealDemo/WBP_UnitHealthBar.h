#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WBP_UnitHealthBar.generated.h"  

class AUnitBase;
class UProgressBar;

UCLASS()
class TFTUNREALDEMO_API UWBP_UnitHealthBar : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Health Bar")
    void SetOwningUnit(AUnitBase* Unit);

    UFUNCTION(BlueprintCallable, Category = "Health Bar")
    void UpdateBars();

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(BlueprintReadOnly, Category = "Health Bar")
    AUnitBase* OwningUnit;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* HealthBar;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* ManaBar;
};