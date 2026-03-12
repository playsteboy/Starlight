// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyCharacter.h"
#include "EnemyCharacter.generated.h"
class UBehaviorTree;
/**
 * 
 */
UCLASS(Abstract)
class STARLIGHT_API AEnemyCharacter : public AMyCharacter
{
	GENERATED_BODY()
	
public:
	AEnemyCharacter();

	UPROPERTY(EditAnywhere)
	UBehaviorTree* BehaviorTreeAsset;
	
};
