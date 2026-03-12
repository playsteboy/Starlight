// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGridCell.h"
#include "BiomeType.h"
FMyGridCell::FMyGridCell()
{
	WorldLocation = FVector::ZeroVector;
	bIsOccupied = false;
	Occupant = nullptr;
	Biome = EBiomeType::Empty;
}

