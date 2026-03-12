// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BiomeType.h"
#include "BiomeData.generated.h"
class UNiagaraSystem; class UNiagaraComponent;
  
 
UCLASS(Abstract, BlueprintType)
class STARLIGHT_API UBiomeData : public UDataAsset
{
	GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere)
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere)
    UMaterialInterface* TileMaterial;

    UPROPERTY(EditAnywhere, Category = "Enemy")
    TSubclassOf<AActor> EnemyMeleeAoEClass;

    UPROPERTY(EditAnywhere, Category = "StarParticles")
    UNiagaraSystem* StarSystem;

	UFUNCTION(BlueprintCallable, Category = "Enemy")
    float GetEnemySpawnRate() const;

    UFUNCTION(BlueprintCallable, Category = "StarParticles")
	float GetStarDensity() const;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:

    UPROPERTY(EditAnywhere, Category = "Enemy")
    float EnemySpawnRate;

    UPROPERTY(EditAnywhere, Category = "StarParticles")
    float StarDensity;
	
};
