// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyCharacter.generated.h"

UCLASS(Abstract)
class STARLIGHT_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	int Health;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void SetHealth(float Amount);

	virtual int GetHealth() const;

	

};
