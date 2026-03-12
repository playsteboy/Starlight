// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyCollectible.h"
#include "HealthPickup.generated.h"
class UBoxComponent; class UStaticMeshComponent;
/**
 * 
 */
UCLASS()
class STARLIGHT_API AHealthPickup : public AMyCollectible
{
	GENERATED_BODY()
	
public:
	AHealthPickup();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PickupMesh;

protected:
	virtual void OverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) override;

private:
	int HealthAmount;
};
