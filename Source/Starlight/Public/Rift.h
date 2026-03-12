// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyPortal.h"
#include "Rift.generated.h"
class UBoxComponent; class AMainCharacter; class UStaticMeshComponent; class AMyGridManager;
/**
 * 
 */
UCLASS()
class STARLIGHT_API ARift : public AMyPortal
{
	GENERATED_BODY()

public:
	ARift();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* RiftMesh;

	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleMapGeneration();

private:
	UFUNCTION()
	virtual void OnZoneEnter(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	AMainCharacter* MC;
	AMyGridManager* GridManager;
	FTimerHandle TimerHandle_Respawn;
	
};
