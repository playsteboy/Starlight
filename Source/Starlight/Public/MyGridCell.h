// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BiomeType.h"
#include "MyGridCell.generated.h"
  
 
USTRUCT()
struct FMyGridCell
{
	GENERATED_BODY()
public:
	FMyGridCell();

	FVector WorldLocation;
	bool bIsOccupied;
	UPROPERTY()
	TWeakObjectPtr<AActor> Occupant;

	EBiomeType Biome;
};
