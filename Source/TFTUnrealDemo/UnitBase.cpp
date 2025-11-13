// UnitBase.cpp
#include "UnitBase.h"

AUnitBase::AUnitBase()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AUnitBase::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    UE_LOG(LogTemp, Log, TEXT("UnitBase initialized!"));
}

void AUnitBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}