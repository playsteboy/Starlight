// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyAIController.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class STARLIGHT_API AEnemyAIController : public AMyAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

protected:

	virtual void OnPossess(APawn* InPawn) override;
	
};
