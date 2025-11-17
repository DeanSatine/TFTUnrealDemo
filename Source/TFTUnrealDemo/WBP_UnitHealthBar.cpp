#include "WBP_UnitHealthBar.h"
#include "UnitBase.h"
#include "Components/ProgressBar.h"

void UWBP_UnitHealthBar::NativeConstruct()
{
    Super::NativeConstruct();
    UpdateBars();
}

void UWBP_UnitHealthBar::SetOwningUnit(AUnitBase* Unit)
{
    OwningUnit = Unit;
    UpdateBars();
}

void UWBP_UnitHealthBar::UpdateBars()
{
    if (!OwningUnit || !HealthBar || !ManaBar)
    {
        return;
    }

    // Update health bar
    float HealthPercent = OwningUnit->CurrentHealth / FMath::Max(OwningUnit->MaxHealth, 1.0f);
    HealthBar->SetPercent(HealthPercent);

    // Update mana bar
    float ManaPercent = OwningUnit->CurrentMana / FMath::Max(OwningUnit->MaxMana, 1.0f);
    ManaBar->SetPercent(ManaPercent);

    UE_LOG(LogTemp, Log, TEXT("Updated bars for %s - HP: %.1f%%, Mana: %.1f%%"),
        *OwningUnit->UnitName, HealthPercent * 100.0f, ManaPercent * 100.0f);
}