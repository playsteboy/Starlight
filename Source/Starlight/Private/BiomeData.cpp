// Fill out your copyright notice in the Description page of Project Settings.


#include "BiomeData.h"

#if WITH_EDITOR
void UBiomeData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UBiomeData, EnemySpawnRate))
    {
        EnemySpawnRate = FMath::Clamp(EnemySpawnRate, 0.0f, 100.0f);
    }
}
#endif

float UBiomeData::GetEnemySpawnRate() const
{
    return EnemySpawnRate;
}

float UBiomeData::GetStarDensity() const
{
    return StarDensity;
}