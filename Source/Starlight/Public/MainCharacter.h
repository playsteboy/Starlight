// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyCharacter.h"
#include "MainCharacter.generated.h"
class UInputAction; class UInputMappingContext; struct FInputActionValue; class AMyGridManager;
/**
 * 
 */
UCLASS()
class STARLIGHT_API AMainCharacter : public AMyCharacter
{
	GENERATED_BODY()

public:
		// Sets default values for this character's properties
	AMainCharacter();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float DeltaTime) override;


	UPROPERTY(BlueprintReadWrite)
	UInputMappingContext* Move_IMC;
	
	UPROPERTY(BlueprintReadWrite)
	UInputAction* Move_IA;

	UFUNCTION(BlueprintCallable)
	virtual int GetHealth() const override;
		
	UFUNCTION(BlueprintCallable)
	int32 GetScore() const;

	UFUNCTION(BlueprintCallable)
	void SetScore(float Amount);

	UFUNCTION(BlueprintCallable)
	virtual void SetHealth(float Amount) override;

	UFUNCTION(BlueprintCallable)
	void SetLightShardCount(int32 Amount);

	UFUNCTION(BlueprintCallable)
	int32 GetLightShardCount() const;

protected:
	virtual void BeginPlay() override;

	int32 Score;

	int32 LightShardCount;

private:
	void Move(const FInputActionValue& Value);

	AMyGridManager* GridManager;

};
