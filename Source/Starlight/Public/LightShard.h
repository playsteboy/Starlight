// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyCollectible.h"
#include "LightShard.generated.h"
class UBoxComponent; class ARift; class AMyGridManager; class UStaticMeshComponent;
/**
 * 
 */
UCLASS()
class STARLIGHT_API ALightShard : public AMyCollectible
{
	GENERATED_BODY()

public:
	ALightShard();

	virtual void OverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	)override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* LightShardMesh;


protected:
	virtual void BeginPlay() override;

private:

	ARift* Rift;
	AMyGridManager* GridManager;


};
