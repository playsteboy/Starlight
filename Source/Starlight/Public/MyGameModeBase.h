// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameModeBase.generated.h"
class AMyGridManager; class ARift;
UCLASS()
class STARLIGHT_API AMyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMyGameModeBase();

	UPROPERTY(EditAnywhere, Category = "Setup")
	TSubclassOf<AMyGridManager> MyGridManagerClass;


	UFUNCTION(BlueprintCallable)
	void FinishGame();

	virtual void RestartPlayer(AController* NewPlayer) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bGameFinished;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	float TimeRemaining;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TSubclassOf<ARift> RiftClass;

	UFUNCTION(BlueprintCallable)
	float GetTimeRemaining() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bPlayerWon;

	UPROPERTY(BlueprintReadOnly, Category = "Score")
	float TotalScore;

	UFUNCTION(BlueprintCallable, Category = "Score")
	float CalculateFinalScore();

	UFUNCTION(BlueprintCallable)
	void SpawnRiftAtPlayerLocation();
protected:
	virtual void BeginPlay() override;

	FTimerHandle TimerHandle_Mission;

	void UpdateTimer();

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float CapsuleHalfHeight;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float TileHeight;
private:

	
	
	AMyGridManager* MyGridManager;

	FVector SpawnLocation;

	FVector PlayerStartLocation;

	float TimeRemainingSave;

	APawn* HandlePlayerSpawn(AController* Player);
};



