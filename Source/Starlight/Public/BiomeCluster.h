// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BiomeType.h"
#include "BiomeBounds.h"
#include "BiomeCluster.generated.h"

USTRUCT()
struct FBiomeCluster
{
    GENERATED_BODY()

    EBiomeType BiomeType;

    TArray<FIntPoint> Tiles;

    FVector Center;

	FBiomeBounds BiomeBounds;
};