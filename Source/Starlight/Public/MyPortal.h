// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyPortal.generated.h"

UCLASS(Abstract)
class STARLIGHT_API AMyPortal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyPortal();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
