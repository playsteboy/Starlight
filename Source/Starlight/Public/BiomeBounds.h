// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BiomeBounds.generated.h"
/**
 * 
 */
USTRUCT()
struct FBiomeBounds
{
	GENERATED_BODY()
public:
	FBiomeBounds();
    int MinX;
    int MaxX;
    int MinY;
    int MaxY;
};
